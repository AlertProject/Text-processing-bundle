using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using LemmaSharp;
using TextLib.TextMining;
using System.Diagnostics;
using System.Text.RegularExpressions;
using GenLib.Text;
using Contextify.Shared.Types;
using Contextify.Shared.Base;
using HtmlAgilityPack;
using System.IO;
using SemWeb;
//using ContextifyServer.Base;

namespace KEUIApp
{
	public partial class KEUI
	{
		private Store _conceptsStore = new MemoryStore();

		private Dictionary<string, string> _sourceCodeToUri = new Dictionary<string, string>();
		AnnotationOntology _ao = null;

		private string _ontologyFileName = "AnnotationOntology.rdf";
		public string IgnoredConceptsFileName = "ignoredLabels.txt";
		public string CustomLemmasFileName = "customLemmas.txt";
		//private Dictionary<string, string> _conceptLabelToUri = new Dictionary<string, string>();
		//private Dictionary<string, string> _conceptLabelNormalizedToUri = new Dictionary<string, string>();
		//private Dictionary<string, string> _conceptUriToDescription = new Dictionary<string, string>();
		//private Dictionary<string, List<string>> _conceptUriToLabels = new Dictionary<string, List<string>>();
		//private Dictionary<string, int> _conceptUriToFrequencyCount = new Dictionary<string, int>();
		
		//private HashSet<string> _codeUrls = new HashSet<string>();		// urls for the source code - files, classes and methods

		List<string> REStackClassAndMethodList = new List<string>() { @"(?<before>(^|\r?\n)#(?<num>[0-9]+).*?) (?<annotate>(?<class>[^: ]+)::(?<method>[^ (]+)[ ]*(?<params>\(.*?\))?)(?<after>[ \n\r])" };
		List<string> REStackMethodAndFileList = new List<string>() { @"(?<before>(^|\r?\n)#(?<num>[0-9]+)[ 0-9a-fx]+ in) (?<annotate>(?<method>[^ (]+)[ ]*(?<params>\(.*?\))?) (?<after>(at|from) (?<file>[^\n\r]*))" };
		List<string> REClassAndMethodList = new List<string>() { @" (?<class>[^: ]+)::(?<method>[^ \(]+)[ ]*(\(.*?\))?" };
		List<string> REBugUrlRefsList = new List<string>() { @"http[s]?://bugs.kde.org/show_bug.cgi\?id=(?<id>[0-9]+)" };
		List<string> REBugReferenceList = new List<string>() { @"(?<before>(^|[\W]+))(?<annotate>bug (?<id>[0-9]+))" };

		string _fileNameREBugUrlRefs = "REBugUrlRefs.txt";
		string _fileNameREStackClassAndMethod = "REStackClassAndMethod.txt";
		string _fileNameREStackMethodAndFile = "REStackMethodAndFile.txt";
		string _fileNameREClassAndMethod = "REClassAndMethod.txt";
		string _fileNameREBugReference = "REBugReference.txt";

		public bool AnnotateWikiPost(HtmlAgilityPack.XmlDocument xmlDoc)
		{
			try
			{
				var eventDataNode = xmlDoc.DocumentNode.SelectSingleNode("//ns1:eventData");
				var wikiNode = eventDataNode.SelectSingleNode("./r2:wikiSensor");

				string subject = GetNodeInnerText(wikiNode, "./r2:title", "");
				string body = GetNodeInnerText(wikiNode, "./r2:rawText", "");

				// annotate the text
				AddNewLines(eventDataNode, 8);
				HtmlNode keuiNode = xmlDoc.CreateElement("s1:keui");
				eventDataNode.AppendChild(keuiNode);

				AddAnnotatedData(keuiNode, subject, "s1:titleAnnotated", "s1:titleConcepts", "s1:titleReferences", 9);
				AddAnnotatedData(keuiNode, body, "s1:rawTextAnnotated", "s1:rawTextConcepts", "s1:rawTextReferences", 9);
			}
			catch (Exception ex)
			{
				AddEvent("Exception while annotating new wiki post: " + ex.Message);
			}
			return false;
		}

		public bool AnnotateCommit(HtmlAgilityPack.XmlDocument xmlDoc)
		{
			try
			{
				var eventDataNode = xmlDoc.DocumentNode.SelectSingleNode("//ns1:eventData");
				var kesiNode = eventDataNode.SelectSingleNode("./s:kesi");

				string comment = GetNodeInnerText(kesiNode, "./s:commitMessageLog", "");
				
				// annotate the text
				AddNewLines(eventDataNode, 8);
				HtmlNode keuiNode = xmlDoc.CreateElement("s1:keui");
				eventDataNode.AppendChild(keuiNode);

				AddAnnotatedData(keuiNode, comment, "s1:commitMessageLogAnnotated", "s1:commitMessageLogConcepts", "s1:commitMessageLogReferences", 9);
			}
			catch (Exception ex)
			{
				AddEvent("Exception while annotating source code: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while annotating source code.", ex);
			}
			return false;
		}

		public bool AnnotateEmail(HtmlAgilityPack.XmlDocument xmlDoc)
		{
			try
			{
				var eventDataNode = xmlDoc.DocumentNode.SelectSingleNode("//ns1:eventData");
				var mlNode = eventDataNode.SelectSingleNode("./r1:mlsensor");

				string subject = GetNodeInnerText(mlNode, "./r1:subject", "");
				string body = GetNodeInnerText(mlNode, "./r1:content", "");

				// annotate the text
				AddNewLines(eventDataNode, 8);
				HtmlNode keuiNode = xmlDoc.CreateElement("s1:keui");
				eventDataNode.AppendChild(keuiNode);

				AddAnnotatedData(keuiNode, subject, "s1:subjectAnnotated", "s1:subjectConcepts", "s1:subjectReferences", 9);
				AddAnnotatedData(keuiNode, body, "s1:contentAnnotated", "s1:contentConcepts", "s1:contentReferences", 9);

				return true;
			}
			catch (Exception ex)
			{
				AddEvent("Exception while annotating email: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while annotating email.", ex);
			}
			return false;
		}

		public bool AnnotateIssueNew(HtmlAgilityPack.XmlDocument xmlDoc)
		{
			try {
				var eventDataNode = xmlDoc.DocumentNode.SelectSingleNode("//ns1:eventData");
				var kesiNode = eventDataNode.SelectSingleNode("./s:kesi");

				// add the keui section
				AddNewLines(eventDataNode, 8);
				HtmlNode keuiNode = xmlDoc.CreateElement("s1:keui");
				eventDataNode.AppendChild(keuiNode);

				// annotate the bug summary
				string description = GetNodeInnerText(kesiNode, "./s:issueDescription", "");
				AddAnnotatedData(keuiNode, description, "s1:issueDescriptionAnnotated", "s1:issueDescriptionConcepts", "s1:issueDescriptionReferences", 9);

				// annotate the bug description
				HtmlNode commentTextNode = xmlDoc.CreateElement("s1:issueComment");
				AddNewLines(keuiNode, 9);
				keuiNode.AppendChild(commentTextNode);
				string comment = GetNodeInnerText(kesiNode, "./s:issueComment/s:commentText", "");
				AddAnnotatedData(commentTextNode, comment, "s1:commentTextAnnotated", "s1:commentTextConcepts", "s1:commentTextReferences", 9);
				return true;
			}
			catch (Exception ex) {
				AddEvent("Exception while annotating IssueNew: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while annotating IssueNew.", ex);
			}
			return false;
		}

		public bool AnnotateIssueUpdate(HtmlAgilityPack.XmlDocument xmlDoc)
		{
			try {
				var eventDataNode = xmlDoc.DocumentNode.SelectSingleNode("//ns1:eventData");
				var kesiNode = eventDataNode.SelectSingleNode("./s:kesi");

				// add the keui section
				AddNewLines(eventDataNode, 8);
				HtmlNode keuiNode = xmlDoc.CreateElement("s1:keui");
				eventDataNode.AppendChild(keuiNode);

				// annotate all the comments
				var commentNodes = kesiNode.SelectNodes("./s:issueComment");
				int commentCount = commentNodes != null ? commentNodes.Count : 0;
				for (int commentN = 0; commentN < commentCount; commentN++) {
					string comment = GetNodeInnerText(kesiNode, string.Format("./s:issueComment[{0}]/s:commentText", commentN + 1), "");
					HtmlNode commentTextNode = xmlDoc.CreateElement("s1:issueComment");
					AddNewLines(keuiNode, 9);
					keuiNode.AppendChild(commentTextNode);
					AddAnnotatedData(commentTextNode, comment, "s1:commentTextAnnotated", "s1:commentTextConcepts", "s1:commentTextReferences", 10);
				}
				return true;
			}
			catch (Exception ex) {
				AddEvent("Exception while annotating new bug post: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while annotating new bug post.", ex);
			}
			return false;
		}
		
		//public bool AnnotateIssueUpdate(HtmlAgilityPack.XmlDocument xmlDoc)
		//{
		//    try
		//    {
		//        var eventDataNode = xmlDoc.DocumentNode.SelectSingleNode("//ns1:eventData");
		//        var kesiNode = eventDataNode.SelectSingleNode("./s:issue");

		//        string comment = GetNodeInnerText(kesiNode, "./s:issueComment/s:commentText", null);
		//        if (comment == null)
		//            return false;

		//        // annotate the text
		//        AddNewLines(eventDataNode, 8);
		//        HtmlNode keuiNode = xmlDoc.CreateElement("s1:keui");
		//        eventDataNode.AppendChild(keuiNode);

		//        AddAnnotatedData(keuiNode, comment, "s1:commentTextAnnotated", "s1:commentTextConcepts", "s1:commentTextReferences", 9);
		//        return true;
		//    }
		//    catch (Exception ex)
		//    {
		//        AddEvent("Exception while annotating new bug comment: " + ex.Message);
		//        GenLib.Log.LogService.LogException("Exception while annotating new bug comment.", ex);
		//    }
		//    return false;
		//}

		public bool AnnotateForumPost(HtmlAgilityPack.XmlDocument xmlDoc)
		{
			try
			{			
				var eventDataNode = xmlDoc.DocumentNode.SelectSingleNode("//ns1:eventData");
				var forumNode = eventDataNode.SelectSingleNode("./r:forumSensor");

				string subject = GetNodeInnerText(forumNode, "./r:subject", "");
				string body = GetNodeInnerText(forumNode, "./r:body", "");
				
				// annotate the text
				AddNewLines(eventDataNode, 8);
				HtmlNode keuiNode = xmlDoc.CreateElement("s1:keui");
				eventDataNode.AppendChild(keuiNode);

				AddAnnotatedData(keuiNode, body, "s1:bodyAnnotated", "s1:bodyConcepts", "s1:bodyReferences", 9);
				AddAnnotatedData(keuiNode, subject, "s1:subjectAnnotated", "s1:subjectConcepts", "s1:subjectReferences", 9);
				
				return true;			// return OuterHtml not InnerHtml, otherwise we don't get the newlines printed correctly
			}
			catch (Exception ex)
			{
				AddEvent("Exception while annotating new forum post: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while annotating new forum post.", ex);
			}
			return false;
		}

		public bool AnnotateTextToAnnotate(HtmlAgilityPack.XmlDocument xmlDoc)
		{
			try
			{
				var eventDataNode = xmlDoc.DocumentNode.SelectSingleNode("//ns1:eventData");
				var textNode = eventDataNode.SelectSingleNode("./s1:generalText");
				string source = GetNodeInnerText(textNode, "./s1:source", "");
				string text = GetNodeInnerText(textNode, "./s1:text", "");
				
				// annotate the text and publish it
				Dictionary<string, double> conceptToWeight = new Dictionary<string, double>();
				HashSet<string> references = new HashSet<string>();
				string annotatedBody = AnnotateText(text, conceptToWeight, references);

				AddNewLines(eventDataNode, 8);
				HtmlNode keuiNode = xmlDoc.CreateElement("s1:keui");
				eventDataNode.AppendChild(keuiNode);

				AddAnnotatedData(keuiNode, text, "s1:textAnnotated", "s1:textConcepts", "s1:textReferences", 9);

				return true;
			}
			catch (Exception ex)
			{
				AddEvent("Exception while annotating text to annotate: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while annotating text to annotate.", ex);
			}
			return false;
		}

		/// <summary>
		/// annotate the <paramref name="text"/> with references to the ontology concepts
		/// create a child of the <paramref name="node"/> named <paramref name="annotatedNodeName"/> and put the annotated text in it
		/// create a child of the <paramref name="node"/> named <paramref name="annotatedConceptsName"/> and put the list of concepts in it
		/// </summary>
		/// <param name="node"></param>
		/// <param name="text"></param>
		/// <param name="annotatedNodeName"></param>
		/// <param name="annotatedConceptsName"></param>
		/// <param name="indentCount"></param>
		/// <returns></returns>
		public Dictionary<string, double> AddAnnotatedData(HtmlNode node, string text, string annotatedNodeName, string annotatedConceptsName, string referencesNodeName, int indentCount)
		{
			Dictionary<string, double> conceptToWeight = new Dictionary<string, double>();
			HashSet<string> references = new HashSet<string>();
			string annotatedText = AnnotateText(text, conceptToWeight, references);
			AddNewLines(node, indentCount);
			AddAnnotatedText(node, annotatedNodeName, annotatedText, putInCDataBlock: true);
			AddNewLines(node, indentCount);
			AddAnnotatedConcepts(node, annotatedConceptsName, conceptToWeight);
			if (references.Count > 0 && !string.IsNullOrEmpty(referencesNodeName))
				AddReferences(node, referencesNodeName, references);
			return conceptToWeight;
		}

		public string DecodeXMLString(string text)
		{
			return text.DecodeXMLString();
		}

		#region Annotation functions
		/// <summary>
		/// annotate the given text and return it
		/// </summary>
		/// <param name="text">input text to be annotated</param>
		/// <param name="conceptToWeight">output: dictionary of detected concepts together with the number of times they appear</param>
		/// <returns>text annotated with concepts from the ontology</returns>
		public string AnnotateText(string text, Dictionary<string, double> conceptToWeight, HashSet<string> references)
		{
			// sometimes a stack trace line is printed in multiple lines. this causes problems when identifying if a line is a part of stack trace or not
			// to fix this problem we remove the extra \n and \r chars from the stack traces
			text = FixMultilineStackTraces(text);

			string annotatedText = AnnotateWithOntologyConcepts(text, conceptToWeight);
			IncreaseConceptFrequencyCounts(conceptToWeight);

			// annotate stack traces with references to classes and methods
			annotatedText = AnnotateStackTraces(annotatedText, conceptToWeight);

			// annotate code clips that contain references to classes and methods
			annotatedText = AnnotateMethodReferences(annotatedText, conceptToWeight);

			// add annotations of references to other bug reports
			annotatedText = AnnotateBugReferences(annotatedText, references);

			return annotatedText;
		}

		/// <summary>
		/// find in the text references to labels of the ontology concepts
		/// annotate these references with the <concept> tag
		/// </summary>
		/// <param name="text"></param>
		/// <param name="conceptToWeight"></param>
		/// <returns></returns>
		public string AnnotateWithOntologyConcepts(string text, Dictionary<string, double> conceptToWeight)
		{
			List<Token> tokens = Tokenization.GetTokens(text);
			StringBuilder outputText = new StringBuilder();
			int i = 0;
			int startIndex, endIndex;
			while (i < tokens.Count) {
				string gram3 = CreateLemmatisedNGram(i, 3, tokens);
				string gram2 = CreateLemmatisedNGram(i, 2, tokens);
				string gram1 = Tokenization.NormalizeText(_ao.GetLemma(tokens[i].Text));
				bool isStackLine = IsTokenInStackTrace(text, tokens, i);
				if (!isStackLine && gram3 != null && _ao.ConceptLabelNormalizedToUri.ContainsKey(gram3) && Tokenization.IsTermCandidate(GetOriginalTerm(i, 3, tokens, text))) {
					AddAnnotatedText(i, 3, _ao.ConceptLabelNormalizedToUri[gram3], tokens, outputText, text);
					AddConcept(conceptToWeight, _ao.ConceptLabelNormalizedToUri[gram3], 1);
					i += 3;
				}
				// if we encounter in the text "bug 1234" then we want to annotate the post with this bug id
				else if (!isStackLine && gram2 != null && Tokenization.NormalizeText(tokens[i].Text) == "bug" && Tokenization.IsNumeric(tokens[i + 1].Text)) {
					// we have found a reference to a bug. We don't annotate the bug here but in the AnnotateBugReferences function. Just add the original term and continue
					outputText.Append(text.Substring(tokens[i].Position, tokens[i].Text.Length));
					i += 1;
				}
				else if (!isStackLine && gram2 != null && _ao.ConceptLabelNormalizedToUri.ContainsKey(gram2) && Tokenization.IsTermCandidate(GetOriginalTerm(i, 2, tokens, text))) {
					AddAnnotatedText(i, 2, _ao.ConceptLabelNormalizedToUri[gram2], tokens, outputText, text);
					AddConcept(conceptToWeight, _ao.ConceptLabelNormalizedToUri[gram2], 1);
					i += 2;
				}
				else if (!isStackLine && _ao.ConceptLabelNormalizedToUri.ContainsKey(gram1)) {
					AddAnnotatedText(i, 1, _ao.ConceptLabelNormalizedToUri[gram1], tokens, outputText, text);
					AddConcept(conceptToWeight, _ao.ConceptLabelNormalizedToUri[gram1], 1);
					i += 1;
				}
				else {
					outputText.Append(text.Substring(tokens[i].Position, tokens[i].Text.Length));
					i += 1;
				}

				// add the text from the last term to the next term - usually a space or some terminating char
				startIndex = tokens[i - 1].Position + tokens[i - 1].Text.Length;
				if (i < tokens.Count)
					endIndex = tokens[i].Position;
				else
					endIndex = text.Length;
				if (endIndex > startIndex)
					outputText.Append(text.Substring(startIndex, endIndex - startIndex));
			}

			return outputText.ToString();
		}

		/// <summary>
		/// Annotate the stack traces with references to class names and methods
		/// for example: #2  0x07e12a12 in QTimerInfoList::timerWait(timeval&) --> #2  0x07e12a12 in <concept id="idOfTheMethod">QTimerInfoList::timerWait(timeval&)</concept>
		/// </summary>
		/// <param name="text"></param>
		/// <returns></returns>
		public string AnnotateStackTraces(string text, Dictionary<string, double> conceptToWeight)
		{
			foreach (string REStackClassAndMethod in REStackClassAndMethodList)
			{
				text = Regex.Replace(text, REStackClassAndMethod, match =>
				{
					try
					{
						foreach (string name in new[] { "class", "method", "params", "before", "after", "annotate" })
						{
							if (!match.Groups[name].Success)
								return match.Value;
						}

						string className = GetCleanClassName(match.Groups["class"].Value);
						string methodName = GetCleanMethodName(match.Groups["method"].Value);
						int paramsCount = GetMethodArgumentCount(match.Groups["params"].Value, false);
						string dictKey = GetKeyForClassMethod(className, methodName, paramsCount);
						if (!_sourceCodeToUri.ContainsKey(dictKey))			// a method and class that we don't know -> we don't annotate
							return match.Value;

						// check if we have found a reference to a class and method inside the text that is already an annotation
						// this should probably never happen but we check nonetheless
						// for example: <concept id='..'>bla class::name</concept>   - in this case we don't annotate the class::name since we would have annotation inside annotation
						int startIndex = match.Index;
						int openingCount = text.Substring(0, startIndex).Count("<concept");
						int endingCount = text.Substring(0, startIndex).Count("</concept>");
						if (openingCount != endingCount)
						{
							AddEventAndLog("A tag is open so we are skipping annotation of the method: " + text);
							return match.Value;
						}

						// weight the concept based on how far down the stack trace it appears. if 0 -> weight = 1, otherwise n -> 1/2^n
						int weight = 0;
						if (match.Groups["num"].Success)
							int.TryParse(match.Groups["num"].Value, out weight);
						if (weight < 0) weight = 0;
						if (weight == 0)
						{
							AddConcept(conceptToWeight, MakeUriLong(_sourceCodeToUri[dictKey]), 1.0);
							return String.Format("{0} <concept id=\"{1}\" weight=\"1\">{2}</concept> {3}", match.Groups["before"].Value, MakeUriLong(_sourceCodeToUri[dictKey]).EncodeXMLString(), match.Groups["annotate"].Value, match.Groups["after"].Value);
						}
						else
						{
							AddConcept(conceptToWeight, MakeUriLong(_sourceCodeToUri[dictKey]), 1.0 / Math.Pow(2, weight));	// add the concept with weight that falls exponentially with the depth of the stack trace
							return String.Format("{0} <concept id=\"{1}\" weight=\"{2:F4}\">{3}</concept> {4}", match.Groups["before"].Value, MakeUriLong(_sourceCodeToUri[dictKey]).EncodeXMLString(), 1.0 / Math.Pow(2, weight), match.Groups["annotate"].Value, match.Groups["after"].Value);
						}
					}
					catch (Exception ex)
					{
						AddEvent("Exception while annotating stack trace: " + ex.Message);
						GenLib.Log.LogService.LogException("Exception while annotating stack trace. match.value =" + match.Value, ex);
						return match.Value;
					}
				}, RegexOptions.IgnoreCase);
			}

			foreach (string REStackMethodAndFile in REStackMethodAndFileList)
			{
				text = Regex.Replace(text, REStackMethodAndFile, match =>
				{
					try
					{
						foreach (string name in new[] { "before", "annotate", "class", "method", "params", "after" })
						{
							if (!match.Groups[name].Success)
								return match.Value;
						}

						string fileName = GetCleanFileName(match.Groups["file"].Value);
						string method = GetCleanMethodName(match.Groups["method"].Value);
						int paramsCount = GetMethodArgumentCount(match.Groups["params"].Value, false);

						string dictKey = GetKeyForFileMethod(fileName, method, paramsCount);
						if (!_sourceCodeToUri.ContainsKey(dictKey))			// a method and class that we don't know -> we don't annotate
							return match.Value;

						// check if we have found a reference to a class and method inside the text that is already an annotation
						// this should probably never happen but we check nonetheless
						// for example: <concept id='..'>bla class::name</concept>   - in this case we don't annotate the class::name since we would have annotation inside annotation
						int startIndex = match.Index;
						int openingCount = text.Substring(0, startIndex).Count("<concept");
						int endingCount = text.Substring(0, startIndex).Count("</concept>");
						if (openingCount != endingCount)
						{
							AddEventAndLog("A tag is open so we are skipping annotation of the method: " + text);
							return match.Value;
						}

						// weight the concept based on how far down the stack trace it appears. if 0 -> weight = 1, otherwise n -> 1/2^n
						int weight = 0;
						int.TryParse(match.Groups["num"].Value, out weight);
						if (weight < 0) weight = 0;
						if (weight == 0)
						{
							AddConcept(conceptToWeight, MakeUriLong(_sourceCodeToUri[dictKey]), 1.0);
							return String.Format("{0} <concept id=\"{1}\" weight=\"1\">{2}</concept> {3}", match.Groups["before"].Value, MakeUriLong(_sourceCodeToUri[dictKey]).EncodeXMLString(), match.Groups["annotate"].Value, match.Groups["ending"].Value);
						}
						else
						{
							AddConcept(conceptToWeight, MakeUriLong(_sourceCodeToUri[dictKey]), 1.0 / Math.Pow(2, weight));	// add the concept with weight that falls exponentially with the depth of the stack trace
							return String.Format("{0} <concept id=\"{1}\" weight=\"{2:F4}\">{3}</concept> {4}", match.Groups["before"].Value, MakeUriLong(_sourceCodeToUri[dictKey]).EncodeXMLString(), 1.0 / Math.Pow(2, weight), match.Groups["annotate"].Value, match.Groups["ending"].Value);
						}
					}
					catch (Exception ex)
					{
						AddEvent("Exception while annotating stack trace: " + ex.Message);
						GenLib.Log.LogService.LogException("Exception while annotating stack trace. match.value =" + match.Value, ex);
						return match.Value;
					}
				}, RegexOptions.IgnoreCase);
			}

			return text;
		}

		public string AnnotateMethodReferences(string text, Dictionary<string, double> conceptToWeight)
		{
			foreach (string REClassAndMethod in REClassAndMethodList)
			{
				text = Regex.Replace(text, REClassAndMethod, match =>
				{
					try
					{
						foreach (string name in new[] { "class", "method" })
						{
							if (!match.Groups[name].Success)
								return match.Value;
						}

						string className = GetCleanClassName(match.Groups["class"].Value);
						string method = GetCleanMethodName(match.Groups["method"].Value);
						string dictKey = String.Format("{0}::{1}", className, method);
						if (!_sourceCodeToUri.ContainsKey(dictKey))			// a method and class that we don't know -> we don't annotate
							return match.Value;

						int startIndex = match.Index;
						int openingCount = text.Substring(0, startIndex).Count("<concept");
						int endingCount = text.Substring(0, startIndex).Count("</concept>");
						if (openingCount != endingCount)
							return match.Value;

						AddConcept(conceptToWeight, MakeUriLong(_sourceCodeToUri[dictKey]), 1.0);
						return String.Format(" <concept id=\"{0}\">{1}</concept>", MakeUriLong(_sourceCodeToUri[dictKey]).EncodeXMLString(), match.Value.TrimStart());
					}
					catch (Exception ex)
					{
						AddEvent("Exception while annotating stack trace: " + ex.Message);
						GenLib.Log.LogService.LogException("Exception while annotating method references. match.value =" + match.Value, ex);
						return match.Value;
					}
				}, RegexOptions.IgnoreCase);
			}
			return text;
		}

		/// <summary>
		/// find occurences of "... bug 1234" and annotate them as "... <reference bugid="1234">bug 1234</reference>"
		/// </summary>
		/// <param name="text">text to annotate</param>
		/// <returns>annotated text</returns>
		public string AnnotateBugReferences(string text, HashSet<string> references)
		{
			foreach (string REBugReference in REBugReferenceList)
			{
				try
				{
					text = Regex.Replace(text, REBugReference, match =>
					{
						try
						{
							foreach (string name in new[] { "before", "id", "annotate" })
							{
								if (!match.Groups[name].Success)
									return match.Value;
							}

							int startIndex = match.Index;
							int openingCount = text.Substring(0, startIndex+1).Count("<concept");
							int endingCount = text.Substring(0, startIndex+1).Count("</concept>");
							if (openingCount != endingCount)
							{
								AddEvent("A tag is open so we are skipping annotation of the bug: " + match.Groups["annotate"].Value);
								GenLib.Log.LogService.LogWarning("A tag is open so we are skipping annotation of the bug. match = " + match.Value);
								return match.Value;
							}

							// find the uri of the bug report
							string bugId = match.Groups["id"].Value;
							string bugUri = "";
							IEnumerable<TagInfoBase> trackerTagIds = MailData.GetTags(_tagIdIssues);
							foreach (var trackerTag in trackerTagIds)
							{
								TagInfoBase tagInfo = MailData.GetTagInfo(trackerTag.TagId, bugId);
								if (tagInfo != null)
									bugUri = tagInfo.TagIdStr;
								if (!string.IsNullOrEmpty(bugUri))
									break;
							}
							if (string.IsNullOrEmpty(bugUri))
							{
								Trace.WriteLine("A reference to an unknown bug was found: " + match.Value);
								bugUri = bugId;
							}
							else
								references.Add(bugUri);
							return String.Format("{0}<reference bugId=\"{1}\" bugUri=\"{2}\">{3}</reference>", match.Groups["before"].Value, bugId, bugUri.EncodeXMLString(), match.Groups["annotate"].Value);
						}
						catch (Exception ex)
						{
							AddEvent("Exception while annotating bug references: " + ex.Message);
							GenLib.Log.LogService.LogException("Exception while annotating bug references. match.value =" + match.Value, ex);
							return match.Value;
						}
					}, RegexOptions.IgnoreCase);
				}
				catch (Exception ex)
				{
					AddEvent("Exception while annotating text: " + ex.Message);
					GenLib.Log.LogService.LogException("Exception while annotating bug references.", ex);
				}
			}

			try
			{
				// find all links that link to some bug reports
				foreach (string REBugUrlRefs in REBugUrlRefsList)
				{
					text = Regex.Replace(text, REBugUrlRefs, match =>
					{
						try
						{
							string bugId = match.Groups["id"].Value;
							string bugUri = "";
							IEnumerable<TagInfoBase> trackerTagIds = MailData.GetTags(_tagIdIssues);
							foreach (var trackerTag in trackerTagIds)
							{
								TagInfoBase tagInfo = MailData.GetTagInfo(trackerTag.TagId, bugId);
								if (tagInfo != null)
									bugUri = tagInfo.TagIdStr;
								if (!string.IsNullOrEmpty(bugUri))
									break;
							}
							if (string.IsNullOrEmpty(bugUri))
								bugUri = bugId;
							else
								references.Add(bugUri);
							return String.Format("<reference bugId=\"{0}\" bugUri=\"{1}\">{2}</reference>", bugId, bugUri.EncodeXMLString(), match.Value);
						}
						catch (Exception ex)
						{
							AddEvent("Exception while annotating bug references: " + ex.Message);
							GenLib.Log.LogService.LogException("Exception while annotating bug references. match.value =" + match.Value, ex);
							return match.Value;
						}
					}, RegexOptions.IgnoreCase);
				}
			}
			catch (Exception ex)
			{
				AddEvent("Exception while annotating text: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while annotating text.", ex);
			}

			return text;
		}

		public void AddNewLines(HtmlNode node, int numberOfTabs)
		{
			if (node == null)
				return;
			string text = Environment.NewLine;
			for (int i = 0; i < numberOfTabs; i++)
				text += "\t";
			HtmlTextNode textNode = node.OwnerDocument.CreateTextNode(text);
			node.AppendChild(textNode);
		}

		void AddAnnotatedText(HtmlNode node, string elementTitle, string annotatedData, bool putInCDataBlock = true)
		{
			if (node == null)
				return;
			XmlDocument doc = (XmlDocument)node.OwnerDocument;

			HtmlNode annotatedNode;
			if (putInCDataBlock)
				annotatedNode = doc.CreateCDataElementInsideWrapperNode(elementTitle, annotatedData);
			else
				annotatedNode = CreateNodeWithTextContent(doc, elementTitle, annotatedData.EncodeXMLString());
			//annotatedNode.Name = elementTitle;
			node.AppendChild(annotatedNode);
		}

		void AddAnnotatedConcepts(HtmlNode node, string elementTitle, Dictionary<string, double> conceptToWeight)
		{
			if (node == null)
				return;
			HtmlNode conceptsNode = node.OwnerDocument.CreateElement(elementTitle);
			foreach (var concept in conceptToWeight)
			{
				HtmlNode conceptNode = node.OwnerDocument.CreateElement("s1:concept");
				HtmlNode uriNode = CreateNodeWithTextContent(node.OwnerDocument, "s1:uri", concept.Key.EncodeXMLString());
				string wgt = (concept.Value == (int)concept.Value) ? ((int)concept.Value).ToString() : String.Format("{0:F4}", concept.Value);
				HtmlNode weightNode = CreateNodeWithTextContent(node.OwnerDocument, "s1:weight", wgt);
				conceptNode.AppendChild(uriNode);
				conceptNode.AppendChild(weightNode);
				conceptsNode.AppendChild(conceptNode);
			}
			node.AppendChild(conceptsNode);
		}

		HtmlNode CreateNodeWithTextContent(HtmlDocument doc, string elementTitle, string text)
		{
			HtmlNode node = doc.CreateElement(elementTitle);
			node.AppendChild(doc.CreateTextNode(text));
			return node;
		}

		void AddReferences(HtmlNode node, string elementTitle, HashSet<string> references)
		{
			if (node == null)
				return;
			HtmlNode referencesNode = node.OwnerDocument.CreateElement(elementTitle);
			foreach (string reference in references)
				referencesNode.AppendChild(CreateNodeWithTextContent(node.OwnerDocument, "s1:referenceUri", reference));
			node.AppendChild(referencesNode);
		}

		/// <summary>
		/// Try locating in the text stack traces. If found, make sure that each line of the stack trace starts with a #.
		/// if a line in the stack trace does not start with a # then we remove the \r\n chars
		/// </summary>
		/// <param name="text"></param>
		/// <returns></returns>
		private string FixMultilineStackTraces(string text)
		{
			try
			{
				foreach (Match match in Regex.Matches(text, @"(?<stack>\r?\n+[ ]*#[0-9]+.*?)(\r?\n\r?\n|$)", RegexOptions.Singleline))
				{
					string originalStack = match.Groups["stack"].Value;
					string editedStack = Regex.Replace(originalStack, @"\r?\n[ ]*(?<val>[^# ])", m => { return " " + m.Groups["val"].Value; }, RegexOptions.Singleline);
					text = text.Replace(originalStack, editedStack);
				}
			}
			catch (Exception ex)
			{
				AddEvent("Exception while fixing stack trace: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while fixing stack trace.", ex);
			}

			return text;
		}

		/// <summary>
		/// return true if the token tokens[index] is in the line that is a part of a stack trace
		/// stack trace is recognized by staring a line with a #[0-9]+
		/// </summary>
		/// <param name="text">original text</param>
		/// <param name="tokens">tokens extracted from text</param>
		/// <param name="index">index of the token in question</param>
		/// <returns></returns>
		private bool IsTokenInStackTrace(string text, List<Token> tokens, int index)
		{
			int tokenPos = tokens[index].Position;
			while (tokenPos > 0 && text[tokenPos] != '\n' && text[tokenPos] != '\r')
				tokenPos--;
			return Regex.IsMatch(text.Substring(tokenPos), @"^\s*#[0-9]+[ \t]+");
		}

		private void AddConcept(Dictionary<string, double> conceptToWeight, string concept, double weight)
		{
			if (conceptToWeight == null) return;
			if (!conceptToWeight.ContainsKey(concept))
				conceptToWeight[concept] = 0.0;
			conceptToWeight[concept] = conceptToWeight[concept] + weight;
		}

		/// <summary>
		/// add the annotations + count tokens from index on
		/// </summary>
		/// <param name="index">the starting index in the tokens list</param>
		/// <param name="count">number of tokens to add</param>
		/// <param name="conceptClass">the name of the concept in the ontology</param>
		/// <param name="tokens">the list of tokens</param>
		/// <param name="outputSentence">output string</param>
		/// <param name="sentence">the original sentence - used to add the original version of the token</param>
		private void AddAnnotatedText(int index, int count, string conceptClass, List<Token> tokens, StringBuilder outputSentence, string sentence)
		{
			outputSentence.Append(string.Format("<concept id=\"{0}\">{1}</concept>", conceptClass, GetOriginalTerm(index, count, tokens, sentence)));
		}

		/// <summary>
		/// Create a n-gram from count tokens in tokens, starting at index.
		/// "Why was this" --> "why is this"
		/// </summary>
		/// <param name="index">index of the first token to use</param>
		/// <param name="count">number of tokens in tokens to use</param>
		/// <param name="tokens">list of tokens</param>
		/// <returns>an n-gram created from the lemmatized tokens</returns>
		private string CreateLemmatisedNGram(int index, int count, List<Token> tokens)
		{
			if (index + count > tokens.Count) return null;
			return String.Join(" ", (from token in tokens.GetRange(index, count) select Tokenization.NormalizeText(_ao.GetLemma(token.Text))).ToArray());
		}

		/// <summary>
		/// Get the original term from the tokens
		/// 
		/// </summary>
		/// <param name="index">index of the first token to consider</param>
		/// <param name="count">number of tokens to consider</param>
		/// <param name="tokens">list of tokens</param>
		/// <param name="text">original text used to create tokens</param>
		/// <returns>original terms that correspond to the considered tokens</returns>
		private string GetOriginalTerm(int index, int count, List<Token> tokens, string text)
		{
			int startIndex = tokens[index].Position;
			int endIndex = tokens[index + count - 1].Position + tokens[index + count - 1].Text.Length;
			return text.Substring(startIndex, endIndex - startIndex);
		}

		
		#endregion

		#region loading annotation data
		/// <summary>
		/// load concept ids and their labels - either from a file or from the ontology
		/// </summary>
		public void InitAnnotationService(string KEUIFolder, string customOntologyFileName = null)
		{
			try
			{
				_ao = new AnnotationOntology();
				_ao.AddEventHandler = AddEvent;

				if (File.Exists(Path.Combine(KEUIFolder, CustomLemmasFileName)))
				{
					List<string> customLemmas = new List<string>();
					foreach (string line in File.ReadAllLines(Path.Combine(KEUIFolder, CustomLemmasFileName)))
						customLemmas.Add(Tokenization.NormalizeText(line));
					_ao.LemmaGen.SetCustomLemmas(customLemmas);
				}
				else
					AddEventAndLog("WARNING: No custom lemmas file was found.");

				string fullPathToOntologyFileName = customOntologyFileName == null ? Path.Combine(KEUIFolder, _ontologyFileName) : customOntologyFileName;
				if (string.IsNullOrEmpty(fullPathToOntologyFileName) || !File.Exists(fullPathToOntologyFileName))
					AddEventAndLog("WARNING: The Alert Ontology file was not found in folder : " + fullPathToOntologyFileName);
				else
					LoadAnnotationOntology(fullPathToOntologyFileName);

				if (File.Exists(Path.Combine(KEUIFolder, IgnoredConceptsFileName)))
				{
					AddEventAndLog("Loading ignored concepts...");
					foreach (var line in File.ReadAllLines(Path.Combine(KEUIFolder, IgnoredConceptsFileName)))
						_ao.IgnoreConcept(line);
				}
				else
					AddEventAndLog("WARNING: No ignored labels file was found.");

				_ao.UpdateSuggestionsDict();
				
				DateTime start = DateTime.Now;
				AddEvent("Building information sources dictionary...");
				foreach (var tagFileInfo in MailData.GetTags(_tagIdSourceCode))
				{
					string fileName = tagFileInfo.TagName;
					if (fileName.Contains('/'))
						fileName = fileName.Substring(fileName.LastIndexOf('/') + 1);
					//if (fileName.Contains('.'))
					//	fileName = fileName.Substring(0, fileName.IndexOf('.'));
					fileName = GetCleanFileName(fileName);
					foreach (var tagModuleInfo in MailData.GetTags(tagFileInfo.TagId))
					{
						string moduleName = tagModuleInfo.TagName;
						moduleName = GetCleanClassName(moduleName);
						foreach (var tagMethodInfo in MailData.GetTags(tagModuleInfo.TagId))
						{
							string methodName = tagMethodInfo.TagName;
							int paramCount = GetMethodArgumentCount(methodName);
							methodName = GetCleanMethodName(methodName);
							InformationSourcesAddFileAndMethod(fileName, methodName, paramCount, tagMethodInfo.TagIdStr);
							InformationSourcesAddClassAndMethod(moduleName, methodName, paramCount, tagMethodInfo.TagIdStr);
						}
					}
				}
				AddEventAndLog("Dictionary built. Time needed: " + (int)(DateTime.Now - start).TotalMilliseconds + " ms");
				AddEventAndLog("Initializing annotator finished.");
			}
			catch (Exception ex)
			{
				AddEvent("Failed to load concepts: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while initializing annotation service.", ex);
			}
		}


		public List<Tuple<string, string>> SuggestConceptsForPrefix(string prefix)
		{
			return _ao.SuggestConceptsForPrefix(prefix);
		}

		public string GetCleanFileName(string fileName)
		{
			if (string.IsNullOrEmpty(fileName))
				return "";
			if (fileName.Contains("/"))
				fileName = fileName.Substring(fileName.LastIndexOf('/') + 1);
			if (fileName.Contains(":"))
				fileName = fileName.Substring(0, fileName.IndexOf(':'));
			fileName = Regex.Replace(fileName, @"\.[\.0-9]+$", m => { return ""; });		// libQtCore.so.4 -> libQtCore.so
			fileName = fileName.Trim();
			return fileName;
		}

		public string GetCleanClassName(string className)
		{
			if (string.IsNullOrEmpty(className))
				return "";
			if (className.Contains(':'))
				className = className.Substring(className.LastIndexOf(':') + 1);
			className = className.Trim();
			return className;
		}

		public string GetCleanMethodName(string methodName)
		{
			if (string.IsNullOrEmpty(methodName))
				return "";
			if (methodName.Contains('('))
				methodName = methodName.Substring(0, methodName.IndexOf('('));
			methodName = methodName.Trim();
			return methodName;
		}

		public int GetMethodArgumentCount(string methodWithArgs, bool removeMethodName = true)
		{
			if (string.IsNullOrEmpty(methodWithArgs))
				return 0;
			string args = null;
			if (removeMethodName) {
				if (!methodWithArgs.Contains('(') || !methodWithArgs.Contains(')'))
					return 0;
				int start = methodWithArgs.IndexOf('(');
				int end = methodWithArgs.IndexOf(')');
				if (end < start)
					return 0;
				args = methodWithArgs.Substring(start + 1, end - start - 1);
			}
			else
				args = methodWithArgs;
			args = args.Trim(" ()".ToCharArray());
			// if we got the method from a stack trace then we have to make sure we don't count the class address as an argument
			// requestTeardown (this=0x81fb718, index=@0xbfa220dc) for example has 1 argument !!
			int thisFound = args.StartsWith("this=") ? 1 : 0;
			if (args == "") return 0;
			return args.Count(',') + 1 - thisFound;
		}

		public void InformationSourcesAddClassAndMethod(string className, string methodName, int paramCount, string uri)
		{
			string key = GetKeyForClassMethod(className, methodName, paramCount);
			_sourceCodeToUri[key] = uri;
		}

		public void InformationSourcesAddFileAndMethod(string fileName, string methodName, int paramCount, string uri)
		{
			string key = GetKeyForFileMethod(fileName, methodName, paramCount);
			_sourceCodeToUri[key] = uri;
		}

		public string GetKeyForClassMethod(string className, string methodName, int paramCount)
		{
			Debug.Assert(className == GetCleanClassName(className), "class name was not normalized: " + className);
			Debug.Assert(methodName == GetCleanMethodName(methodName), "method name was not normalized: " + methodName);
			if (methodName.Contains('('))
				methodName = methodName.Substring(0, methodName.IndexOf('('));
			methodName = methodName.Trim();
			return String.Format("{0}::{1}/{2}", className, methodName, paramCount);
		}

		public string GetKeyForFileMethod(string fileName, string methodName, int paramCount)
		{
			Debug.Assert(fileName == GetCleanFileName(fileName), "file name was not normalized: " + fileName);
			Debug.Assert(methodName == GetCleanMethodName(methodName), "method name was not normalized: " + methodName);
			return String.Format("{0}|{1}/{2}", fileName, methodName, paramCount);
		}
		#endregion

		#region loading/saving
		public void LoadSettings(string folder)
		{
			if (File.Exists(Path.Combine(folder, _fileNameREBugUrlRefs)))
			{
				string[] lines = File.ReadAllLines(Path.Combine(folder, _fileNameREBugUrlRefs));
				REBugUrlRefsList.Clear();
				foreach (string line in lines)
				{
					if (!string.IsNullOrEmpty(line))
						REBugUrlRefsList.Add(line);
				}
			}

			if (File.Exists(Path.Combine(folder, _fileNameREStackClassAndMethod)))
			{
				string[] lines = File.ReadAllLines(Path.Combine(folder, _fileNameREStackClassAndMethod));
				REStackClassAndMethodList.Clear();
				foreach (string line in lines)
				{
					if (!string.IsNullOrEmpty(line))
						REStackClassAndMethodList.Add(line);
				}
			}

			if (File.Exists(Path.Combine(folder, _fileNameREStackMethodAndFile)))
			{
				string[] lines = File.ReadAllLines(Path.Combine(folder, _fileNameREStackMethodAndFile));
				REStackMethodAndFileList.Clear();
				foreach (string line in lines)
				{
					if (!string.IsNullOrEmpty(line))
						REStackMethodAndFileList.Add(line);
				}
			}

			if (File.Exists(Path.Combine(folder, _fileNameREBugReference)))
			{
				string[] lines = File.ReadAllLines(Path.Combine(folder, _fileNameREBugReference));
				REBugReferenceList.Clear();
				foreach (string line in lines)
				{
					if (!string.IsNullOrEmpty(line))
						REBugReferenceList.Add(line);
				}
			}

			if (File.Exists(Path.Combine(folder, _fileNameREClassAndMethod)))
			{
				string[] lines = File.ReadAllLines(Path.Combine(folder, _fileNameREClassAndMethod));
				REClassAndMethodList.Clear();
				foreach (string line in lines)
				{
					if (!string.IsNullOrEmpty(line))
						REClassAndMethodList.Add(line);
				}
			}
		}

		public void SaveSettings(string folder)
		{
			try { File.WriteAllLines(Path.Combine(folder, _fileNameREBugUrlRefs), REBugUrlRefsList); }
			catch { }

			try { File.WriteAllLines(Path.Combine(folder, _fileNameREStackClassAndMethod), REStackClassAndMethodList); }
			catch { }

			try { File.WriteAllLines(Path.Combine(folder, _fileNameREStackMethodAndFile), REStackMethodAndFileList); }
			catch { }

			try { File.WriteAllLines(Path.Combine(folder, _fileNameREBugReference), REBugReferenceList); }
			catch { }

			try { File.WriteAllLines(Path.Combine(folder, _fileNameREClassAndMethod), REClassAndMethodList); }
			catch { }
		}		
		#endregion

		#region Loading/updating/saving annotation ontology
		private string _annotationOntologyFileName = null;
		public void LoadAnnotationOntology(string fileName)
		{
			_annotationOntologyFileName = fileName;
			_ao.LoadAnnotationOntologyFromFile(_annotationOntologyFileName);
		}

		public void SaveAnnotationOntology()
		{
			if (!string.IsNullOrEmpty(_annotationOntologyFileName))
				_ao.SaveAnnotationOntology(_annotationOntologyFileName);
		}

		public void AddNewRDFData(string rdfData, bool removeExistingDataForSubjects = true)
		{
			_ao.AddNewRDFData(rdfData, removeExistingDataForSubjects);
		}

		public string GetAnnotationOntologyRDF(bool includeComment, bool includeLinksTo)
		{
			return _ao.GetAnnotationOntologyRDF(includeComment, includeLinksTo);
		}

		public void IncreaseConceptFrequencyCounts(Dictionary<string, double> conceptToWeight)
		{
			// we watch that we don't make concepts too popular if they appear only in a few posts
			foreach (string key in conceptToWeight.Keys)
				_ao.IncreaseConceptFrequencyCount(key, 1);
				//_ao.IncreaseConceptFrequencyCount(key, (int) conceptToWeight[key]);
		}

		#endregion

		#region helper functions for ui
		public delegate void SetLastItemIdDelegate(int itemId);
		public delegate void IncreaseProcessedCountDelegate();
		public delegate void AddEventDelegate(string text);

		public SetLastItemIdDelegate SetLastItemIdHandler = null;
		public IncreaseProcessedCountDelegate IncreaseProcessedCountHandler = null;
		private AddEventDelegate _addEventHandler = null;

		public void SetAddEventHandler(AddEventDelegate addEventHandler)
		{
			_addEventHandler = addEventHandler;
		}

		private void SetLastItemId(int itemId)
		{
			if (SetLastItemIdHandler != null)
				SetLastItemIdHandler(itemId);
			else
				Trace.WriteLine("Last item id " + itemId);
		}

		private void IncreaseProcessedCount()
		{
			if (IncreaseProcessedCountHandler != null)
				IncreaseProcessedCountHandler();
		}

		private void AddEventAndLog(string text)
		{
			AddEvent(text);
			GenLib.Log.LogService.LogInfo(text);
		}

		private void AddEvent(string text)
		{
			if (_addEventHandler != null)
				_addEventHandler(text);
			else
				Trace.WriteLine(text);
		}
		#endregion
	}
}
