/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2019 Sartura Ltd.
 *
 * Author: Domagoj Pintaric <domagoj.pintaric@sartura.hr>
 *
 * https://www.sartura.hr/
 */

// #include "uv_curlm_driver.h"
// call uv_curlm_driver_init()
// attach curl easy handles using curl_multi_add_handle(curl_multi, <curl_easy_handle_name>);
// implement curl_multi_info_check() and call curl_multi_info_read()
// call uv_curlm_driver_clean()

#include <stdbool.h>

#include <uv.h>
#include <curl/curl.h>

#include "debug.h"
#include "memory.h"

static int curl_debug_cb(CURL *handle, curl_infotype type, char *data, size_t size, void *userp);
static void curl_multi_timer_start_cb(CURLM *handle, long timeout_ms, void *userp);
static void curl_multi_timer_cb(uv_timer_t *handle);
static int curl_socket_poll_start_cb(CURL *handle, curl_socket_t curl_socket, int action, void *userp, void *socketp);
static void curl_socket_poll_cb(uv_poll_t *handle, int error, int events);
static void curl_socket_poll_free_cb(uv_handle_t *handle);
static void curl_multi_info_check(void);

static CURLM *curl_multi = NULL;
static uv_timer_t curl_multi_timer = {0};

static int uv_curlm_driver_init(void)
{
	curl_multi = curl_multi_init();
	if (curl_multi == NULL) {
		_error("failed to init curl multi hadndle");
		return -1;
	}
	curl_multi_setopt(curl_multi, CURLMOPT_TIMERFUNCTION, curl_multi_timer_start_cb);
	curl_multi_setopt(curl_multi, CURLMOPT_SOCKETFUNCTION, curl_socket_poll_start_cb);
	uv_timer_init(uv_default_loop(), &curl_multi_timer);

	return 0;
}

static int uv_curlm_driver_clean(void)
{
	if (curl_multi) {
		curl_multi_cleanup(curl_multi);
		curl_multi = NULL;
	}

	if (uv_has_ref((uv_handle_t *) &curl_multi_timer) && uv_is_closing((uv_handle_t *) &curl_multi_timer) == false) {
		uv_close((uv_handle_t *) &curl_multi_timer, NULL);
	}

	return 0;
}

static int curl_debug_cb(CURL *handle, curl_infotype type, char *data, size_t size, void *userp)
{
	switch (type) {
		case CURLINFO_TEXT:
			__debug("%s%.*s", "=== Info: ", (int) size - 1, data);
			break;
		case CURLINFO_HEADER_IN:
			__debug("%s%.*s", "<=== Header in: ", (int) size - 1, data);
			break;
		case CURLINFO_HEADER_OUT:
			__debug("%s%.*s", "===> Header out: \n", (int) size - 1, data);
			break;
#ifdef CURL_DEBUG_DATA_IN
		case CURLINFO_DATA_IN:
			__debug("%s%.*s", "<=== Data in:\n", (int) size, data);
			__debug("");
			break;
#endif
#ifdef CURL_DEBUG_DATA_OUT
		case CURLINFO_DATA_OUT:
			__debug("%s%.*s", "===> Data out:\n", (int) size, data);
			__debug("");
			break;
#endif
		default:
			return 0;
	}

	return 0;
}

static void curl_multi_timer_start_cb(CURLM *handle, long timeout_ms, void *userp)
{
	___debug("curl_multi_timer_start_cb: %ld", timeout_ms);

	// don't call the timer anymore
	// ref: https://curl.haxx.se/libcurl/c/CURLMOPT_TIMERFUNCTION.html
	if (timeout_ms == -1) {
		uv_timer_stop(&curl_multi_timer);
		return;
	}

	uv_timer_start(&curl_multi_timer, curl_multi_timer_cb, (uint64_t) timeout_ms, 0);
}

static void curl_multi_timer_cb(uv_timer_t *handle)
{
	___debug("curl_multi_timer_cb");

	curl_multi_socket_action(curl_multi, CURL_SOCKET_TIMEOUT, 0, &(int){0});
	curl_multi_info_check();
}

static int curl_socket_poll_start_cb(CURL *curl_easy, curl_socket_t curl_socket, int action, void *userp, void *socketp)
{
	___debug("curl_socket_poll_start_cb");

	// NOTE:
	// - curl_socket_poll is "self contained"
	// - created, driven and destroyed by libcurl via this callback

	uv_poll_t *curl_socket_poll = NULL;

	if (socketp) {
		curl_socket_poll = (uv_poll_t *) socketp;
	} else {
		// create curl_socket_poll and attach curl_socket to it's data pointer for usage in curl_socket_poll_cb()
		curl_socket_poll = xcalloc(1, sizeof(uv_poll_t));
		curl_socket_poll->data = xcalloc(1, sizeof(curl_socket_t));
		*((curl_socket_t *) curl_socket_poll->data) = curl_socket;
		uv_poll_init_socket(uv_default_loop(), curl_socket_poll, curl_socket);
		curl_multi_assign(curl_multi, curl_socket, curl_socket_poll);
	}

	uv_poll_stop(curl_socket_poll);
	switch (action) {
		case CURL_POLL_IN:
			___debug("curl poll in");
			uv_poll_start(curl_socket_poll, UV_READABLE, curl_socket_poll_cb);
			break;
		case CURL_POLL_OUT:
			___debug("curl poll out");
			uv_poll_start(curl_socket_poll, UV_WRITABLE, curl_socket_poll_cb);
			break;
		case CURL_POLL_INOUT:
			___debug("curl poll inout");
			uv_poll_start(curl_socket_poll, UV_READABLE | UV_WRITABLE, curl_socket_poll_cb);
			break;
		case CURL_POLL_REMOVE:
			// destroy curl_socket_poll
			___debug("curl poll remove");
			uv_close((uv_handle_t *) curl_socket_poll, curl_socket_poll_free_cb);
			curl_multi_assign(curl_multi, curl_socket, NULL);
			break;
		default:
			_error("unreachable");
			abort();
	}

	return 0;
}

static void curl_socket_poll_cb(uv_poll_t *handle, int error, int events)
{
	___debug("curl_socket_poll_cb");

	int flags = 0;

	if (error < 0) {
		flags = CURL_CSELECT_ERR;
	}

	if (!error && (events & UV_READABLE)) {
		flags |= CURL_CSELECT_IN;
	}

	if (!error && (events & UV_WRITABLE)) {
		flags |= CURL_CSELECT_OUT;
	}

	curl_multi_socket_action(curl_multi, *((curl_socket_t *) handle->data), flags, &(int){0});
	curl_multi_info_check();
}

static void curl_socket_poll_free_cb(uv_handle_t *handle)
{
	___debug("curl_socket_poll_free_cb");

	FREE_SAFE(handle->data);
	FREE_SAFE(handle);
}
