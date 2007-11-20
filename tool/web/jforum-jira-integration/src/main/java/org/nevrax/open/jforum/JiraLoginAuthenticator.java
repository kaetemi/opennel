package org.nevrax.open.jforum;

import java.util.Locale;
import java.util.Map;

import net.jforum.dao.UserDAO;
import net.jforum.entities.User;
import net.jforum.exceptions.ForumException;
import net.jforum.sso.LoginAuthenticator;
import net.jforum.util.MD5;
import net.jforum.util.preferences.SystemGlobals;

import org.apache.axis.AxisFault;

import com.atlassian.jira.rpc.exception.RemoteAuthenticationException;
import com.atlassian.jira.rpc.soap.beans.RemoteUser;
import com.atlassian.jira.rpc.soap.jirasoapservice_v2.JirasoapserviceV2SoapBindingStub;

/**
 * TODO list: group/role mapping between JIRA and JForum?
 */
public class JiraLoginAuthenticator implements LoginAuthenticator {

	private JirasoapserviceV2SoapBindingStub service = null;

	private LoginAuthenticator fallbackAuthenticator = null;

	private UserDAO userDAO = null;

	public JiraLoginAuthenticator() {
		String endpoint = SystemGlobals
				.getValue(JiraConfigKeys.JIRA_SOAP_ENDPOINT);
		try {
			this.service = new JirasoapserviceV2SoapBindingStub(
					new java.net.URL(endpoint), null);
		} catch (Exception e) {
			throw new ForumException("set property '"
					+ JiraConfigKeys.JIRA_SOAP_ENDPOINT
					+ "' to the SOAP endpoint "
					+ "of JIRA (and enable the RPC-plugin).", e);
		}
		String fallbackClass = SystemGlobals
				.getValue(JiraConfigKeys.JIRA_SOAP_FALLBACK);
		if (fallbackClass != null)
			try {
				this.fallbackAuthenticator = (LoginAuthenticator) Class
						.forName(fallbackClass).newInstance();
			} catch (Exception e) {
				// ignore
			}
	}

	public void setUserModel(UserDAO dao) {
		this.userDAO = dao;
		if (this.fallbackAuthenticator != null)
			this.fallbackAuthenticator.setUserModel(dao);
	}

	private User handleFallback(Exception e, String username, String password,
			Map extraParams) {
		// fall back to default authenticator in case of
		// remoting problems, if present
		if (this.fallbackAuthenticator != null)
			return this.fallbackAuthenticator.validateLogin(username, password,
					extraParams);
		else
			// otherwise tell about the problem
			throw new ForumException(e);
	}

	public User validateLogin(String username, String password, Map extraParams) {
		try {
			// log in with the credentials
			String secToken = this.service.login(username, password);
			// grab user information
			RemoteUser jiraUser = this.service.getUser(secToken, username);
			// and log out again
			boolean result = this.service.logout(secToken);
			// if log out worked ok (which means the other methods
			// must have, too), try to load user by the name
			if (result) {
				User forumUser = this.userDAO.selectByName(username);
				// and either auto create a new user
				if (forumUser == null) {
					forumUser = new User();
					forumUser.setEmail(jiraUser.getEmail());
					forumUser.setViewEmailEnabled(false);
					forumUser.setUsername(jiraUser.getName());
					forumUser.setPassword(MD5.crypt(password));
					// TODO is this way correct?
					forumUser.setLang(Locale.ENGLISH.getISO3Language());
					this.userDAO.addNew(forumUser);
				} else { // or refresh the password of the existing one
					forumUser.setPassword(MD5.crypt(password));
					this.userDAO.update(forumUser);
				}
				return forumUser;
			}
		} catch (RemoteAuthenticationException rae) {
			// this results in a standard login error
			return null;
		} catch (AxisFault af) {
			// communication problems and other JIRA faults trigger
			// the usage of a fallback solution, if it is defined
			return handleFallback(af, username, password, extraParams);
		} catch (Exception e) {
			// non-remote problems shouldn't happen, but if they do,
			// make sure you see them ...
			throw new ForumException(e);
		}
		return null;
	}
}
