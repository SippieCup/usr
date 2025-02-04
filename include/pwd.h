/*-
 * Copyright (c) 1989, 1993
 *    The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*-
 * Portions Copyright(C) 1995, Jason Downs.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _PWD_H_
#define _PWD_H_

#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

struct passwd {
  char* pw_name;
  char* pw_passwd;
  uid_t pw_uid;
  gid_t pw_gid;
#ifdef __LP64__
  char* pw_gecos;
#else
  /* Note: On LP32, we define pw_gecos to pw_passwd since they're both NULL. */
# define pw_gecos pw_passwd
#endif
  char* pw_dir;
  char* pw_shell;
};

struct passwd* getpwnam(const char* __name);
struct passwd* getpwuid(uid_t __uid);

/* Note: Android has thousands and thousands of ids to iterate through */

#if __ANDROID_API__ >= 26
struct passwd* getpwent(void) __INTRODUCED_IN(26);

void setpwent(void) __INTRODUCED_IN(26);
#endif /* __ANDROID_API__ >= 26 */



#if __ANDROID_API__ >= 12
int getpwnam_r(const char* __name, struct passwd* __pwd, char* __buf, size_t __n, struct passwd** __result) __INTRODUCED_IN(12);
int getpwuid_r(uid_t __uid, struct passwd* __pwd, char* __buf, size_t __n, struct passwd** __result) __INTRODUCED_IN(12);
#endif /* __ANDROID_API__ >= 12 */

int access(const char* __path, int __mode);

static void android_setup_pwd(struct passwd* pw) {
	char const* result = "/data/data/com.termux/files/usr/bin/login";
	if (result == NULL || access(result, /*X_OK*/1) == -1) {
		pw->pw_shell = "/data/data/com.termux/files/usr/bin/bash";
	} else {
		pw->pw_shell = (char*) result;
	}
	pw->pw_dir = "/data/data/com.termux/files/home";
	pw->pw_passwd = "*";
#ifdef __LP64__
	pw->pw_gecos = ""; /* Avoid NULL field. */
#endif
}

static struct passwd* android_polyfill_getpwuid(uid_t t) {
       struct passwd* pw = getpwuid(t);
       if (pw == NULL) return NULL;
       android_setup_pwd(pw);
       return pw;
}

static struct passwd* android_polyfill_getpwnam(const char* name) {
       struct passwd* pw = getpwnam(name);
       if (pw == NULL) return NULL;
       android_setup_pwd(pw);
       return pw;
}

static int android_polyfill_getpwuid_r(uid_t uid,
                                       struct passwd *pwd,
                                       char *buffer,
                                       size_t bufsize,
                                       struct passwd **result) {
       int ret = getpwuid_r(uid, pwd, buffer, bufsize, result);
       if (ret != 0) return ret;
       android_setup_pwd(pwd);
       return 0;
}

#define getpwnam android_polyfill_getpwnam
#define getpwuid android_polyfill_getpwuid
#define getpwuid_r android_polyfill_getpwuid_r
static void endpwent(void) { /* Do nothing. */ }

__END_DECLS

#endif
