package org.nevrax.open.jforum;

public class JiraConfigKeys {

	/**
	 * URL of the JIRA webservice. Required example:
	 * "http://localhost:7080/rpc/soap/jirasoapservice-v2".
	 */
	public final static String JIRA_SOAP_ENDPOINT = "jira.soap.endpoint";

	/**
	 * classname of the fallback LoginAuthenticator. Optional i.e.:
	 * net.jforum.sso.DefaultLoginAuthenticator.
	 */
	public final static String JIRA_SOAP_FALLBACK = "jira.soap.fallback";

	/**
	 * Name of the cookie which contains the credentials,
	 * e.g. "seraph.os.cookie".
	 */
	public final static String JIRA_LOGIN_COOKIE_NAME = "jira.login.cookie.name";

}
