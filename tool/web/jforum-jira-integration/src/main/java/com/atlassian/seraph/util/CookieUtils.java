/**
 Copyright (c) 2002-2005, Atlassian Software Systems Pty Ltd All rights reserved.

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.
 * Neither the name of Atlassian Software Systems Pty Ltd nor the names of
 its contributors may be used to endorse or promote products derived from this
 software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
package com.atlassian.seraph.util;

import org.apache.log4j.Category;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.Cookie;

/**
 * Seraph utility class for dealing with cookies
 * 
 * Includes code from Jive 1.2.4 (released under the Apache license)
 */
public class CookieUtils {
	private static final Category log = Category.getInstance(CookieUtils.class);

	// the key which represents the list of cookies to add for this request (in
	// case of redirect)
	public static final String COOKIES_TO_SEND = "atlassian.core.web.cookies.unsent";

	// Character used to separate username and password in persistent cookies.
	// 0x13 == "Device Control 3" non-printing ASCII char. Unlikely to appear in
	// a username
	private static final char DELIMITER = 0x13;

	// "Tweakable" parameters for the cookie encoding. NOTE: changing these
	// and recompiling this class will essentially invalidate old cookies.
	private final static int ENCODE_XORMASK = 0x5A;

	private final static char ENCODE_CHAR_OFFSET1 = 'C';

	private final static char ENCODE_CHAR_OFFSET2 = 'i';

	/**
	 * Invalidate the specified cookie and delete it from the response object.
	 * 
	 * @param response
	 *            The HttpServletResponse object, known as "response" in a JSP
	 *            page.
	 * @param cookieName
	 *            The name of the cookie you want to delete.
	 * @param path
	 *            of the path the cookie you want to delete.
	 */
	public static void invalidateCookie(HttpServletRequest request,
			HttpServletResponse response, String cookieName, String path) {
		log.debug("CookieUtils.invalidateCookie " + cookieName + " for path "
				+ path);
		setCookie(request, response, cookieName, null, 0, path);
	}

	/**
	 * Invalidate the specified cookie and delete it from the response object.
	 * Deletes only cookies mapped against the root "/" path. Otherwise use
	 * {@link #invalidateCookie(HttpServletRequest, HttpServletResponse, String, String)}
	 * 
	 * @param response
	 *            The HttpServletResponse object, known as "response" in a JSP
	 *            page.
	 * @param cookieName
	 *            The name of the cookie you want to delete.
	 * @see #invalidateCookie(HttpServletRequest, HttpServletResponse, String,
	 *      String)
	 */
	public static void invalidateCookie(HttpServletRequest request,
			HttpServletResponse response, String cookieName) {
		invalidateCookie(request, response, cookieName, "/");
	}

	/**
	 * Returns the specified Cookie object, or null if the cookie does not
	 * exist.
	 * 
	 * @param request
	 *            The HttpServletRequest object, known as "request" in a JSP
	 *            page.
	 * @param name
	 *            the name of the cookie.
	 * @return the Cookie object if it exists, otherwise null.
	 */
	public static Cookie getCookie(HttpServletRequest request, String name) {
		Cookie cookies[] = request.getCookies();
		if (cookies == null || name == null || name.length() == 0) {
			return null;
		}
		// Otherwise, we have to do a linear scan for the cookie.
		for (int i = 0; i < cookies.length; i++) {
			if (cookies[i].getName().equals(name)) {
				return cookies[i];
			}
		}
		return null;
	}

	/**
	 * Sets a cookie
	 * 
	 * This will also put the cookie in a list of cookies to send with this
	 * request's response (so that in case of a redirect occurring down the
	 * chain, the first filter will always try to set this cookie again)
	 */
	public static Cookie setCookie(HttpServletRequest request,
			HttpServletResponse response, String name, String value,
			int maxAge, String path) {
		log.debug("CookieUtils.setCookie " + name + ":" + value);
		Cookie cookie = new Cookie(name, value);
		cookie.setMaxAge(maxAge);
		cookie.setPath(path);
		response.addCookie(cookie);

		return cookie;
	}

	/**
	 * Returns the value of the specified cookie as a String. If the cookie does
	 * not exist, the method returns null.
	 * 
	 * @param request
	 *            the HttpServletRequest object, known as "request" in a JSP
	 *            page.
	 * @param name
	 *            the name of the cookie
	 * @return the value of the cookie, or null if the cookie does not exist.
	 */
	public static String getCookieValue(HttpServletRequest request, String name) {
		Cookie cookie = getCookie(request, name);
		if (cookie != null) {
			return cookie.getValue();
		}
		return null;
	}

	/**
	 * Builds a cookie string containing a username and password.
	 * <p>
	 * 
	 * Note: with open source this is not really secure, but it prevents users
	 * from snooping the cookie file of others and by changing the XOR mask and
	 * character offsets, you can easily tweak results.
	 * 
	 * @param username
	 *            The username.
	 * @param password
	 *            The password.
	 * @return String encoding the input parameters, an empty string if one of
	 *         the arguments equals <code>null</code>.
	 */
	public static String encodePasswordCookie(String username, String password) {
		return encodePasswordCookie(username, password, new String(new char[] {
				DELIMITER, ENCODE_CHAR_OFFSET1, ENCODE_CHAR_OFFSET2 }));
	}

	/**
	 * Builds a cookie string containing a username and password, using offsets
	 * to customise the encoding.
	 * <p>
	 * 
	 * Note: with open source this is not really secure, but it prevents users
	 * from snooping the cookie file of others and by changing the XOR mask and
	 * character offsets, you can easily tweak results.
	 * 
	 * @param username
	 *            The username.
	 * @param password
	 *            The password.
	 * @param encoding
	 *            A String used to customise cookie encoding (only the first 3
	 *            characters are used)
	 * @return String encoding the input parameters, an empty string if one of
	 *         the arguments equals <code>null</code>.
	 */
	public static String encodePasswordCookie(String username, String password,
			String encoding) {
		StringBuffer buf = new StringBuffer();
		if (username != null && password != null) {
			char offset1 = (encoding != null && encoding.length() > 1) ? encoding
					.charAt(1)
					: ENCODE_CHAR_OFFSET1;
			char offset2 = (encoding != null && encoding.length() > 2) ? encoding
					.charAt(2)
					: ENCODE_CHAR_OFFSET2;

			byte[] bytes = (username + DELIMITER + password).getBytes();
			int b;

			for (int n = 0; n < bytes.length; n++) {
				b = bytes[n] ^ (ENCODE_XORMASK + n);
				buf.append((char) (offset1 + (b & 0x0F)));
				buf.append((char) (offset2 + ((b >> 4) & 0x0F)));
			}
		}
		return buf.toString();
	}

	/**
	 * Decodes a cookie string containing a username and password.
	 * 
	 * @param cookieVal
	 *            The cookie value.
	 * @return String[] containing the username at index 0 and the password at
	 *         index 1, or <code>{ null, null }</code> if cookieVal equals
	 *         <code>null</code> or the empty string.
	 */
	public static String[] decodePasswordCookie(String cookieVal) {
		return decodePasswordCookie(cookieVal, new String(new char[] {
				DELIMITER, ENCODE_CHAR_OFFSET1, ENCODE_CHAR_OFFSET2 }));
	}

	/**
	 * Decodes a cookie string containing a username and password.
	 * 
	 * @param cookieVal
	 *            The cookie value.
	 * @param encoding
	 *            A String used to customise cookie encoding (only the first 3
	 *            characters are used) - should be the same string you used to
	 *            encode the cookie!
	 * @return String[] containing the username at index 0 and the password at
	 *         index 1, or <code>{ null, null }</code> if cookieVal equals
	 *         <code>null</code> or the empty string.
	 */
	public static String[] decodePasswordCookie(String cookieVal,
			String encoding) {
		// check that the cookie value isn't null or zero-length
		if (cookieVal == null || cookieVal.length() <= 0) {
			return null;
		}

		char offset1 = (encoding != null && encoding.length() > 1) ? encoding
				.charAt(1) : ENCODE_CHAR_OFFSET1;
		char offset2 = (encoding != null && encoding.length() > 2) ? encoding
				.charAt(2) : ENCODE_CHAR_OFFSET2;

		// decode the cookie value
		char[] chars = cookieVal.toCharArray();
		byte[] bytes = new byte[chars.length / 2];
		int b;
		for (int n = 0, m = 0; n < bytes.length; n++) {
			b = chars[m++] - offset1;
			b |= (chars[m++] - offset2) << 4;
			bytes[n] = (byte) (b ^ (ENCODE_XORMASK + n));
		}
		cookieVal = new String(bytes);
		int pos = cookieVal.indexOf(DELIMITER);
		String username = (pos < 0) ? "" : cookieVal.substring(0, pos);
		String password = (pos < 0) ? "" : cookieVal.substring(pos + 1);

		return new String[] { username, password };
	}
}
