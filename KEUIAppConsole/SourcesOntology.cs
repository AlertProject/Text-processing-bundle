using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Contextify;
using ContextifyServer.Base;
using Contextify.Shared.Types;
using Contextify.Shared.Base;
using System.Diagnostics;
using GenLib.Text;

namespace KEUIApp
{
	public class SourcesOntology
	{
		public MailData MailData = null;
		Dictionary<string, List<int>> _filePrefixToTagId = new Dictionary<string, List<int>>();
		Dictionary<string, List<int>> _modulePrefixToTagId = new Dictionary<string, List<int>>();
		Dictionary<string, List<int>> _methodPrefixToTagId = new Dictionary<string, List<int>>();

		public SourcesOntology(MailData mailData)
		{
			MailData = mailData;
		}

		public int AddFileTagIfNotExisting(string fileName, string fileUri, int parentTagId)
		{
			TagInfoBase existingTag = MailData.GetTagInfo(fileUri);
			if (existingTag != null)
				return existingTag.TagId;
			int fileTagId = MailData.CreateTagIfNotExisting(fileName, fileUri, parentTagId);
			existingTag = MailData.GetTagInfo(fileTagId);
			UpdateSuggestionsForTag(existingTag, _filePrefixToTagId);
			return existingTag.TagId;
		}

		public int AddModuleTagIfNotExisting(string moduleName, string moduleUri, int parentTagId)
		{
			TagInfoBase existingTag = MailData.GetTagInfo(moduleUri);
			if (existingTag != null)
				return existingTag.TagId;
			int moduleTagId = MailData.CreateTagIfNotExisting(moduleName, moduleUri, parentTagId);
			existingTag = MailData.GetTagInfo(moduleTagId);
			UpdateSuggestionsForTag(existingTag, _modulePrefixToTagId);
			return existingTag.TagId;
		}

		public int AddMethodTagIfNotExisting(string methodName, string methodUri, int parentTagId)
		{
			TagInfoBase existingTag = MailData.GetTagInfo(methodUri);
			if (existingTag != null)
				return existingTag.TagId;
			int methodTagId = MailData.CreateTagIfNotExisting(methodName, methodUri, parentTagId);
			existingTag = MailData.GetTagInfo(methodTagId);
			UpdateSuggestionsForTag(existingTag, _methodPrefixToTagId);
			return existingTag.TagId;
		}

		public List<int> SuggestFilesForPrefix(string prefix)
		{
			try
			{
				if (string.IsNullOrEmpty(prefix))
					return new List<int>();
				prefix = prefix.ToLower(System.Globalization.CultureInfo.InvariantCulture);
				prefix = prefix.Substring(0, Math.Min(prefix.Length, _prefixConceptMaxSize));
				if (_filePrefixToTagId.ContainsKey(prefix))
					return _filePrefixToTagId[prefix];
			}
			catch (Exception ex)
			{
				AddEvent("Exception while retrieving file suggestions for prefix " + prefix + ". " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while retrieving file suggestions for prefix " + prefix + ". ", ex);
			}
			return new List<int>();
		}

		public List<int> SuggestModulesForPrefix(string prefix)
		{
			try
			{
				if (string.IsNullOrEmpty(prefix))
					return new List<int>();
				prefix = prefix.ToLower(System.Globalization.CultureInfo.InvariantCulture);
				prefix = prefix.Substring(0, Math.Min(prefix.Length, _prefixConceptMaxSize));
				if (_modulePrefixToTagId.ContainsKey(prefix))
					return _modulePrefixToTagId[prefix];
			}
			catch (Exception ex)
			{
				AddEvent("Exception while retrieving module suggestions for prefix " + prefix + ". " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while retrieving module suggestions for prefix " + prefix + ". ", ex);
			}
			return new List<int>();
		}

		public List<int> SuggestMethodsForPrefix(string prefix)
		{
			try
			{
				if (string.IsNullOrEmpty(prefix))
					return new List<int>();
				prefix = prefix.ToLower(System.Globalization.CultureInfo.InvariantCulture);
				prefix = prefix.Substring(0, Math.Min(prefix.Length, _prefixConceptMaxSize));
				if (_methodPrefixToTagId.ContainsKey(prefix))
					return _methodPrefixToTagId[prefix];
			}
			catch (Exception ex)
			{
				AddEvent("Exception while retrieving method suggestions for prefix " + prefix + ". " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while retrieving method suggestions for prefix " + prefix + ". ", ex);
			}
			return new List<int>();
		}

		public void UpdateSourcesSuggestionsDict(int TagIdSourceCode)
		{
			try
			{
				_filePrefixToTagId = new Dictionary<string, List<int>>();
				_modulePrefixToTagId = new Dictionary<string, List<int>>();
				_methodPrefixToTagId = new Dictionary<string, List<int>>();

				foreach (TagInfoBase fileTag in MailData.GetTags(TagIdSourceCode))
				{
					UpdateSuggestionsForTag(fileTag, _filePrefixToTagId);
					foreach (TagInfoBase moduleTag in MailData.GetTags(fileTag.TagId))
					{
						UpdateSuggestionsForTag(moduleTag, _modulePrefixToTagId);
						foreach (TagInfoBase methodTag in MailData.GetTags(moduleTag.TagId))
						{
							UpdateSuggestionsForTag(methodTag, _methodPrefixToTagId);
						}
					}
				}
			}
			catch (Exception ex)
			{
				AddEvent("Exception while updating suggestion dict for sources ontology: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while updating suggestion dict for sources ontology. ", ex);
			}
			
		}

		private const int _prefixConceptSuggestionCount = 20;		// max number of concepts that can be suggested for a given prefix
		private const int _prefixConceptMaxSize = 6;				// max length of the prefix considered for suggestion
		private void UpdateSuggestionsForTag(TagInfoBase tag, Dictionary<string, List<int>> dict)
		{
			if (tag == null)
				return;
			string label = tag.TagName;
			label = label.ToLower();
			label = Text.ReplaceUnicodeCharsWithAscii(label);
			if (label.Contains('\\'))
					label = label.Substring(label.LastIndexOf('\\')+1);
				if (label.Contains('/'))
					label = label.Substring(label.LastIndexOf('/')+1);
			int tagId = tag.TagId;
			for (int i = 1; i <= _prefixConceptMaxSize; i++)
			{
				// if label is too short then break
				if (label.Length < i) break;
				// normalize prefix
				string labelPrefix = label.Substring(0, i);
				// create the suggestion list if not existing yet
				if (!dict.ContainsKey(labelPrefix))
					dict[labelPrefix] = new List<int>();
				// if we already have max number of suggestions for this prefix then do nothing
				if (dict[labelPrefix].Count > _prefixConceptSuggestionCount)
					continue;
				// add the tagid for the prefix
				dict[labelPrefix].Add(tagId);
			}
		}

		#region helper functions
		public delegate void AddEventDelegate(string text);
		public AddEventDelegate AddEventHandler = null;

		private void AddEventAndLog(string text)
		{
			AddEvent(text);
			GenLib.Log.LogService.LogInfo(text);
		}

		private void AddEvent(string text)
		{
			if (AddEventHandler != null)
				AddEventHandler(text);
			else
				Trace.WriteLine(text);
		}
		#endregion
	}
}
