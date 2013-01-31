using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Diagnostics;

using System.Globalization;
using System.IO;
using System.Xml;

namespace GenLib
{
	public class Email
	{
		public struct Link
		{
			public string Url;
			public string Text;
			public bool IsWebLink
			{
				get { return Url.ToLower().StartsWith("http") || Url.ToLower().StartsWith("ftp"); }
			}
		}

		/// <summary>
		/// remove all the re:, fw: fwd:, aw:, ... from the beginning of the subjecty
		/// </summary>
		/// <param name="subject"></param>
		/// <returns></returns>
		public static string GetCleanSubject(string subject)
		{
			if (subject == null) return "";
			subject = Regex.Replace(subject, @"^([\s]*[a-z]{2,3}:)*\s", "", RegexOptions.IgnoreCase);
			return subject;
		}

		private static string _invalidEmailChars = @"[;!\\""#$*:,<> \']";
		public static string NormalizeEmail(string email)
		{
			if (!String.IsNullOrEmpty(email))
			{
				email = email.ToLower();
				email = email.Trim();
#if !SILVERLIGHT
				if (Regex.IsMatch(email, _invalidEmailChars))
					Trace.WriteLine(String.Format("Email {0} seems to contain invalid characters.", email));
#endif
				//// replace characters that are not supposed to be in the emails with empty string
				//foreach (string s in new string[] { ":", ",", "<", ">", " ", "\"", "'" })
				//    email = email.Replace(s, "");
			}
			return email;
		}

		// check if the email seems to be valid
		public static bool IsValidEmail(string email)
		{
			if (String.IsNullOrEmpty(email)) return false;
			if (!email.Contains("@")) return false;
			if (Regex.IsMatch(email, _invalidEmailChars))
				return false;
			return true;
		}

		/// <summary>
		/// Find links in the html text
		/// </summary>
		/// <param name="html">html to search for the links</param>
		/// <param name="ignoredLinksList">the list of regular expressions for links that should be ignored. If a link matches any of these expressions it will not be added to the resulting list of links</param>
		/// <returns></returns>
		public static List<Link> FindLinks(string html, List<string> ignoredLinksList = null)
		{
			List<Link> list = new List<Link>();
			if (string.IsNullOrEmpty(html)) 
				return list;

			// Find all matches in file.
			//MatchCollection m1 = Regex.Matches(html, @"(<a.*?>.*?</a>)", RegexOptions.IgnoreCase | RegexOptions.Singleline);
			foreach (Match m in Regex.Matches(html, @"(<a\s+[^>]*href\s*=\s*""(?<url>[^""]*)""[^>]*>(?<text>.*?)</a\s*>)", RegexOptions.IgnoreCase | RegexOptions.Singleline))
			{
				try
				{
					if (m.Groups["url"] == null || m.Groups["text"] == null)
						continue;
					string url = m.Groups["url"].Value;
					string text = m.Groups["text"].Value;

					// check if the link points to a site that we wish to ignore. if yes, then ignore the link
					if (ignoredLinksList != null && ignoredLinksList.Any(ignore => Regex.IsMatch(url, ignore)))
						continue;

					//if (cleanText != null && !(cleanText.Contains(i.Href) || cleanText.Contains(DecodeXMLString(i.Href))))
					//    break;		// once we come to a link which is not contained in the cleanText we stop searching for links
				
					// Remove inner tags from text.
					text = Regex.Replace(text, @"<[^>]*>", "", RegexOptions.Singleline);
					text = text.Replace("\n", " ").Replace("\r", ""); // remove newlines
					text = Regex.Replace(text, "  +", " ");	// remove multiple spaces
					text = text.Trim();

					list.Add(new Link() { Url = url, Text = text});
				}
				catch (Exception ex)
				{
#if !SILVERLIGHT
					Trace.WriteLine(ex.Message);
#endif
				}
			}
			
			return list;
		}

		public static string GetCleanOutlookMailBody(string body)
		{
			// remove links from the body
			Regex reg = new Regex("(?<hyper>HYPERLINK \"(.*?)\")");
			foreach (Match m in reg.Matches(body))
				body = body.Replace(m.Groups["hyper"].Value, " ");
			body.Replace("HYPERLINK \\l \"", " ");		// some links in the text appear as this string and we have to remove it this way

			body = body.Replace((char)160, ' ');	// remove &nbsp characters (chars with int code 160)
			//body = body.Replace("\r\n", " ");
			//body = Regex.Replace(body, "(\n|\r|\t)", " ", RegexOptions.Singleline);	// remove new lines
			//body = Regex.Replace(body, @"(\r\n\s*)*\r\n", "\r\n");		// replace multiple occurences of newline with a single newline

			// remove useless character sequences
			//List<string> regExExpressions = new List<string>() { "___+", "---+" };
			//foreach (string s in regExExpressions)
			//    body = Regex.Replace(body, s, " ");
			
			// remove extra spaces
			body = Regex.Replace(body, "  +", " ");

			return body;
		}

		/// <summary>
		/// return only the last message send in the email
		/// </summary>
		/// <param name="body"></param>
		/// <returns></returns>
		public static string GetLastEmailBody(string body)
		{
			if (body == null) return null;
			string filteredBody = body.ToString();

			//// first remove the conversations that are two steps back (notice the >> in the regex)
			filteredBody = Regex.Replace(filteredBody, @"^[>]*[-]+\s*(Original|Forwarded) Message\s*[-]+\r\n(\s*\r\n|>.*\r\n)*", "", RegexOptions.Multiline | RegexOptions.IgnoreCase);
			filteredBody = Regex.Replace(filteredBody, @"^[>]*On .*,.* wrote:\r\n(\s*\r\n|>.*\r\n)*", "", RegexOptions.Multiline | RegexOptions.IgnoreCase);
			filteredBody = Regex.Replace(filteredBody, @"^[>]*[0-9:/-]+ .* <.+@.+>\r\n(\s*\r\n|>.*\r\n)*", "", RegexOptions.Multiline | RegexOptions.IgnoreCase);
			filteredBody = Regex.Replace(filteredBody, @"^[>]*On .*, .* <.+@.+>\s*wrote[:]?\r\n(\s*\r\n|>.*\r\n)*", "", RegexOptions.Multiline | RegexOptions.IgnoreCase);

			// remove the replies that just start with From: ... Subject... etc and don't use > in the reply.
			// this regex is the only one that cleans out everything after the matching string - other regexes just remove the lines with > in them
			filteredBody = Regex.Replace(filteredBody, @"^[>]*From:.*\r\n([>]*(Date|Sender|Subject|CC|BCC|Sent):.*\r\n)*[>]*To:.*\r\n(.|\n|\r)*", "", RegexOptions.Multiline | RegexOptions.IgnoreCase);

			//filteredBody = Regex.Replace(filteredBody, @"^[-]+\s*Original Message\s*[-]+(.|\n|\r)*", "", RegexOptions.Multiline | RegexOptions.IgnoreCase);
			//filteredBody = Regex.Replace(filteredBody, @"^-[-]+\s*Forwarded message\s*-[-]+(.|\n|\r)*", "", RegexOptions.Multiline | RegexOptions.IgnoreCase);
			//filteredBody = Regex.Replace(filteredBody, "^On .*,.* wrote:(.|\n|\r)*", "", RegexOptions.Multiline);															// On 26.2.2010 14:43, X Y wrote: 

			//List<string> names = new List<string>();
			//names.AddRange(from person in participants select Regex.Escape(person.DisplayName));
			//names.AddRange(from person in participants select Regex.Escape(person.MailAddress));

			//filteredBody = Regex.Replace(filteredBody, "^(" + String.Join("|", names.ToArray()) + ")\\s+wrote(.|\n|\r)*", "", RegexOptions.Multiline);															// On 26.2.2010 14:43, X Y wrote: 
			//filteredBody = Regex.Replace(filteredBody, "^From:.*(" + String.Join("|", names.ToArray()) + ")(.|\n|\r)*", "", RegexOptions.Multiline | RegexOptions.IgnoreCase);						// From: Gregor Leban [mailto:gleban@gmail.com] On Behalf Of Gregor Leban
			//filteredBody = Regex.Replace(filteredBody, "^[0-9:/-]+ .*(" + String.Join("|", names.ToArray()) + ")(.|\n|\r)*", "", RegexOptions.Multiline | RegexOptions.IgnoreCase);				// 2010/3/19 Gregor Leban <HYPERLINK \"mailto:gleban@gmail.com\"gleban@gmail.com>

			filteredBody = Regex.Replace(filteredBody, @"^\s*", "");		// remove starting white space
			filteredBody = Regex.Replace(filteredBody, @"\s*$", "");		// remove ending white space

			if (string.IsNullOrEmpty(filteredBody))
				return body;		// if we filtered out all the content from the email then simply return the full email (this can happen if we simply forward an email)
			else
				return filteredBody;
		}

		// regular expression that matches all except words and spaces
		private static Regex _keepWordsAndSpaces = new Regex(@"[^\w\s]*");
		
		// put the name into lower case and replace all special characters with normal characters
		public static string GetCleanNormalizedName(string name)
		{
			name = name.ToLower();
			name = _keepWordsAndSpaces.Replace(name, "");
			char[] from = { 'š', 'đ', 'č', 'ć', 'ž' };
			char[] to = { 's', 'd', 'c', 'c', 'z' };
			for (int i = 0; i < from.Length; i++)
				name = name.Replace(from[i], to[i]);
			return name;
		}

		public static string RemoveInvalidCharsFromEmail(string email)
		{
			if (string.IsNullOrEmpty(email)) return email;
			return Regex.Replace(email, @"[:;!""*,<> ']*", "");
			//return email.Replace(FacebookEncoding, "").Replace(LinkedInEncoding, ""); 
		}

		// remove the specified characters from the given name
		public static string GetCleanName(string name, string removedChars = "'\"|$#!/=?*{}")
		{
			if (String.IsNullOrEmpty(name))
				return name;
			return String.Join("", name.Split(removedChars.ToCharArray(), StringSplitOptions.RemoveEmptyEntries));
		}
	}
}
