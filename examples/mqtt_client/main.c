/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019, Erik Moqvist
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * This file is part of the Async project.
 */

#include "async.h"
#include "publisher.h"

static const char certificate[] = (
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDCjCCAfICCQCoXfSQydXTczANBgkqhkiG9w0BAQsFADBHMQswCQYDVQQGEwJT\n"
    "RTETMBEGA1UECAwKU29tZS1TdGF0ZTEOMAwGA1UECgwFU2ltYmExEzARBgNVBAMM\n"
    "CmZvb2Jhci5vcmcwHhcNMTcwMTA3MTgxOTQ3WhcNMTgwMTA3MTgxOTQ3WjBHMQsw\n"
    "CQYDVQQGEwJTRTETMBEGA1UECAwKU29tZS1TdGF0ZTEOMAwGA1UECgwFU2ltYmEx\n"
    "EzARBgNVBAMMCmZvb2Jhci5vcmcwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\n"
    "AoIBAQDugoc9amynDB/ODJzh2aqB6cnubSJEPlgB87jNG0akcbmUFt6BhPhMPSoP\n"
    "esHWl5OWscoga0cKrPURmMVVhfaeZLQGmrv5N4/liVlwae1n0gUEruX4d6MqSSDW\n"
    "3C/WKjCn647udZwyzCvyrvPOq0qAzaxR4EFRdwjSEPO5sDw2zxeTjGW2WxaH9PEu\n"
    "C8vaNqTsLYl3YBkR3zVCbuQXTQhStsv3gT4Yhz2wJpY0yyWyDiaBkGKpdxJQiNAd\n"
    "x5JKSqtRshlYZM3+cdKLywNoYUnezp6Wm4mzz09TCFv+esJ0h7/6pMdVjhxLsAg5\n"
    "ZbZyrtNIapN07AjIJS4qjkJ/HUC3AgMBAAEwDQYJKoZIhvcNAQELBQADggEBAGHS\n"
    "U5AvDESzTNoak5HHx166bp5+bvECvJg45GMDGhqKeFoOlR34u2b+wyTm/piz3nYJ\n"
    "12kn+BbG/PwGfndL0kZYRz46fY8Rf+MxCFfcglweDJhA6ULNpera11OC35Q/lKn5\n"
    "M6w6eQkZMB4VqwigvDGHGpXRTIhJHHoR2VFBFGoPTLrXilChUpiXi9DmuYXJ/19x\n"
    "sxOVwvvO/m/6g68G+uZYUoCsQsKllM2fgdNLTzbYvnFtsq5QnZS8m7CoZgEy2c3m\n"
    "VrrPsfwmyhwejDawjz2epSLNXaaDeSz4g1cQes+oehaA1IwFfKhb9tdiWUm48VuU\n"
    "oaFZ8e1HofYUy+65pws=\n"
    "-----END CERTIFICATE-----\n");

int main()
{
    struct async_t async;
    struct publisher_t publishers[2];
    struct async_ssl_context_t ssl_context;

    async_ssl_module_init();
    async_init(&async);
    async_set_runtime(&async, async_runtime_create());
    publisher_init(&publishers[0],
                   "mqtt-client-example",
                   1883,
                   NULL,
                   &async);
    async_ssl_context_init(&ssl_context, async_ssl_protocol_tls_v1_0_t);
    async_ssl_context_load_verify_location(&ssl_context, &certificate[0]);
    async_ssl_context_set_verify_mode(&ssl_context,
                                      async_ssl_verify_mode_cert_none_t);
    publisher_init(&publishers[1],
                   "mqtt-client-example-ssl",
                   8883,
                   &ssl_context,
                   &async);
    async_run_forever(&async);

    return (0);
}
