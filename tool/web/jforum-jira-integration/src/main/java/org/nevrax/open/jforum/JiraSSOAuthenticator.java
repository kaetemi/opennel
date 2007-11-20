package org.nevrax.open.jforum;

import javax.servlet.http.Cookie;

import net.jforum.ControllerUtils;
import net.jforum.context.RequestContext;
import net.jforum.entities.UserSession;
import net.jforum.exceptions.ForumException;
import net.jforum.sso.SSO;
import net.jforum.util.preferences.ConfigKeys;
import net.jforum.util.preferences.SystemGlobals;

import com.atlassian.jira.rpc.exception.RemoteAuthenticationException;
import com.atlassian.jira.rpc.soap.beans.RemoteUser;
import com.atlassian.jira.rpc.soap.jirasoapservice_v2.JirasoapserviceV2SoapBindingStub;
import com.atlassian.seraph.util.CookieUtils;

/**
 * SSO implementation to work together with JIRA. It's based on the default
 * Seraph implementation to encode the username cookie. It also requires the
 * JIRA RPC plugin to be enabled (see JIRA docs how this is done) where it uses
 * the SOAP interface to validate the Seraph cookie information.
 * 
 * Required properties to be set in the WEB-INF/config/SystemGlobals.properties
 * file:
 * <ul>
 * <li><code>jira.soap.endpoint</code> which defines the URI to access the
 * JIRA SOAP service.</li>
 * <li><code>jira.login.cookie.name</code> name of the cookie which contains
 * the user name someone is logged in with at JIRA.</li>
 * </ul>
 */
public class JiraSSOAuthenticator implements SSO {

	JirasoapserviceV2SoapBindingStub service = null;

	public JiraSSOAuthenticator() {
		String endpoint = SystemGlobals
				.getValue(JiraConfigKeys.JIRA_SOAP_ENDPOINT);
		try {
			this.service = new JirasoapserviceV2SoapBindingStub(
					new java.net.URL(endpoint), null);
		} catch (Exception e) {
			throw new ForumException("set property '"
					+ JiraConfigKeys.JIRA_SOAP_ENDPOINT
					+ "' to the SOAP endpoint "
					+ "of JIRA (and enable the RPC-plugin in JIRA).", e);
		}
	}

	public String authenticateUser(RequestContext requestCtx) {
		String user = null;

		// get Seraph cookie
		Cookie myCookie = ControllerUtils.getCookie(SystemGlobals
				.getValue(JiraConfigKeys.JIRA_LOGIN_COOKIE_NAME));
		// and decode cookie infos if they are present
		if (myCookie != null) {
			String[] credentials = CookieUtils.decodePasswordCookie(myCookie
					.getValue());
			if (credentials.length == 2) {
				try {
					// crosscheck the cookie info:
					// - log in with the credentials
					String secToken = this.service.login(credentials[0],
							credentials[1]);
					// - grab user information
					RemoteUser jiraUser = this.service.getUser(secToken,
							credentials[0]);
					// - and log out again
					boolean result = this.service.logout(secToken);
					// if log out worked ok (which means the other methods
					// must have, too), try to load user by the name
					if (result) {
						// set user name properly
						user = credentials[0];
						// and fill in some additional info into the session
						// context to create "fitting" data for new users.
						requestCtx.getSessionContext().setAttribute("password",
								credentials[1]);
						requestCtx.getSessionContext().setAttribute("email",
								jiraUser.getEmail());
					}
				} catch (RemoteAuthenticationException rae) {
					// failed login attempt
				} catch (Exception e) {
					// everything else hints at something broken
					throw new ForumException(e);
				}
			}
		}

		// if a user could be digged up, set the JForum cookie
		if (user != null)
			// set the JForum login cookie
			ControllerUtils.addCookie(SystemGlobals
					.getValue(ConfigKeys.COOKIE_NAME_DATA), user);
		// return the user name (which can be null)
		return user;
	}

	// Cut & paste code from the JForum website, spiced up with
	// a check against the JIRA/Seraph cookie.
	public boolean isSessionValid(UserSession session, RequestContext requestCtx) {
		String remoteUser = null;
		Cookie SSOCookie = ControllerUtils.getCookie(SystemGlobals
				.getValue(ConfigKeys.COOKIE_NAME_DATA));
		if (SSOCookie != null)
			remoteUser = SSOCookie.getValue(); // jforum username

		// get Seraph cookie
		Cookie myCookie = ControllerUtils.getCookie(SystemGlobals
				.getValue(JiraConfigKeys.JIRA_LOGIN_COOKIE_NAME));
		// render session invalid if the JIRA/Seraph cookie is missing
		if (myCookie == null)
			return false;

		String[] credentials = CookieUtils.decodePasswordCookie(myCookie
				.getValue());
		if (credentials.length == 0) // this shouldn't happen
			return false;
		// things are not in sync if both cookies refer to
		// different user names
		if (remoteUser != null && !remoteUser.equals(credentials[0]))
			return false;

		// user has since logged out
		if (remoteUser == null
				&& session.getUserId() != SystemGlobals
						.getIntValue(ConfigKeys.ANONYMOUS_USER_ID)) {
			return false;

			// user has since logged in
		} else if (remoteUser != null
				&& session.getUserId() == SystemGlobals
						.getIntValue(ConfigKeys.ANONYMOUS_USER_ID)) {
			return false;

			// user has changed user
		} else if (remoteUser != null
				&& !remoteUser.equals(session.getUsername())) {
			return false;
		}
		return true; // myapp user and forum user the same
	}

}
