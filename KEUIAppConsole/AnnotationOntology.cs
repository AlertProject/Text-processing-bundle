using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using SemWeb;
//using LemmaSharp;
using TextLib.TextMining;
using System.Diagnostics;
using GenLib.Text;

namespace KEUIApp
{
	public class AnnotationOntology
	{
		public static string RDFS = "http://www.w3.org/2000/01/rdf-schema#";
		public static string RDF = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";
		public static string DBPedia = "http://dbpedia.org/ontology/";
		public static string OWL = "http://www.w3.org/2002/27/owl#";
		public static string AiLab = "http://ailab.ijs.si/alert/predicate/";
		public static string Purl = "http://purl.org/dc/terms/";

		public static Entity RelRdfsLabel = RDFS + "label";
		public static Entity RelRdfsComment = RDFS + "comment";
		public static Entity RelDbAbstract = DBPedia + "abstract";
		public static Entity RelRdfValue = RDF + "value";
		public static Entity RelLinksTo = AiLab + "linksTo";
		public static Entity RelRdfType = RDF + "type";
		public static Entity RelOwlSameAs = OWL + "sameAs";
		public static Entity RelPurlIsPartOf = Purl + "isPartOf";
		public static Entity RelPurlCreator = Purl + "creator";
		
		private Store _conceptsStore = null;

		public Dictionary<string, string> ConceptLabelToUri = new Dictionary<string, string>();
		public Dictionary<string, string> ConceptLabelNormalizedToUri = new Dictionary<string, string>();
		public Dictionary<string, string> ConceptUriToDescription = new Dictionary<string, string>();
		public Dictionary<string, List<string>> ConceptUriToLabels = new Dictionary<string, List<string>>();
		private Dictionary<string, int> _conceptUriToFrequencyCount = new Dictionary<string, int>();

		private TextLib.Lemmatization.LemmaGen _lemmaGen = null;
		public TextLib.Lemmatization.LemmaGen LemmaGen { get { return _lemmaGen; } }

		public AnnotationOntology()
		{
			_lemmaGen = new TextLib.Lemmatization.LemmaGen();
		}

		public string GetLemma(string word)
		{
			Trace.Assert(_lemmaGen != null);
			return _lemmaGen.GetLemma(word);
		}

		// ignore the term or a set of terms
		// the specified term should never be annotated
		public void IgnoreConcept(string label)
		{
			if (string.IsNullOrEmpty(label))
				return;
			string labelNormalized = Tokenization.NormalizeText(label);
			if (ConceptLabelToUri.ContainsKey(labelNormalized))
				ConceptLabelToUri.Remove(labelNormalized);

			string lemmaPartsJoined = GetLemmatizedText(label);
			if (ConceptLabelNormalizedToUri.ContainsKey(lemmaPartsJoined))
				ConceptLabelNormalizedToUri.Remove(lemmaPartsJoined);
		}

		public int GetConceptCount()
		{
			return ConceptUriToDescription.Count;
		}

		public int GetConceptFrequencyCount(string conceptUri)
		{
			if (conceptUri != null && _conceptUriToFrequencyCount.ContainsKey(conceptUri))
				return _conceptUriToFrequencyCount[conceptUri];
			return 0;
		}

		public string GetConceptUri(string conceptLabel)
		{
			string labelNormalized = Tokenization.NormalizeText(conceptLabel);
			if (ConceptLabelToUri.ContainsKey(labelNormalized))
				return  ConceptLabelToUri[labelNormalized];
			return null;
		}

		public List<string> GetConceptLabels(string conceptUri)
		{
			if (conceptUri != null && ConceptUriToLabels.ContainsKey(conceptUri))
				return ConceptUriToLabels[conceptUri];
			return new List<string>();
		}
		
		public string GetConceptLabel(string conceptUri)
		{
			if (conceptUri != null && ConceptUriToLabels.ContainsKey(conceptUri))
			{
				foreach (string label in ConceptUriToLabels[conceptUri])
				{
					if (!string.IsNullOrEmpty(label))
						return label;
				}
			}
			return "";
		}

		public string GetConceptDescription(string conceptUri)
		{
			if (conceptUri != null && ConceptUriToDescription.ContainsKey(conceptUri))
				return ConceptUriToDescription[conceptUri];
			return "";
		}
		
		public void LoadAnnotationOntologyFromFile(string filename)
		{
			_conceptsStore = new MemoryStore();
			AddEventAndLog("Loading annotation ontology from file: " + filename);
			_conceptsStore.Import(RdfReader.LoadFromUri(new Uri(filename)));
			UpdateDictionaries();
		}

		public void LoadAnnotationOntologyFromString(string rdfData)
		{
			_conceptsStore = new MemoryStore();
			RdfXmlReader reader = new RdfXmlReader(new System.IO.StringReader(rdfData));
			_conceptsStore.Import(reader);
			UpdateDictionaries();
		}

		public void UpdateDictionaries()
		{
			try
			{
				// ////////////////////////////////////////////////////////////////////////
				// Load the ontology
				// load the concept uris and labels. use them to create dict label -> uri

				HashSet<string> seenLabels = new HashSet<string>();
				foreach (Statement s in _conceptsStore.Select(new Statement(null, RelRdfsLabel, null)))
				{
					//split the name and compute lemmas. add the lemmas into the dictionary, not the conceptName
					string subject = s.Subject.Uri;
					string label = ((Literal)s.Object).Value;
					string labelNormalized = Tokenization.NormalizeText(label);
					if (seenLabels.Contains(labelNormalized))
					{
						if (s.Subject.Uri != ConceptLabelToUri[labelNormalized])
							AddEventAndLog(String.Format("The same label {0} is assigned to multiple concepts:\n{1} and \n{2}\n------------", labelNormalized, subject, ConceptLabelToUri[labelNormalized]));
						else
							_conceptsStore.Remove(s);		// forget the statement - its just a duplicate
						continue;
					}
					seenLabels.Add(labelNormalized);
					ConceptLabelToUri[labelNormalized] = s.Subject.Uri;
					string labelLemmaPartsJoined = GetLemmatizedText(label);
					ConceptLabelNormalizedToUri[labelLemmaPartsJoined] = s.Subject.Uri;
					if (!ConceptUriToLabels.ContainsKey(s.Subject.Uri))
						ConceptUriToLabels[s.Subject.Uri] = new List<string>();
					ConceptUriToLabels[s.Subject.Uri].Add(label);
					//_conceptUriToFrequencyCount[s.Subject.Uri] = 0;
				}

				foreach (Statement s in _conceptsStore.Select(new Statement(null, RelRdfsComment, null)))
					ConceptUriToDescription[s.Subject.Uri] = ((Literal)s.Object).Value;

				foreach (Statement s in _conceptsStore.Select(new Statement(null, RelDbAbstract, null)))
					ConceptUriToDescription[s.Subject.Uri] = ((Literal)s.Object).Value;

				foreach (Statement s in _conceptsStore.Select(new Statement(null, RelRdfValue, null)))
					_conceptUriToFrequencyCount[s.Subject.Uri] = int.Parse(((Literal)s.Object).Value);

				AddEventAndLog(String.Format("Annotation ontology is now cached. Ontology contains {0} labels.", ConceptLabelToUri.Count));
			}
			catch (Exception ex)
			{
				AddEvent("Exception while loading the annotation ontology. Error: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while loading the annotation ontology", ex);
			}
		}

		/// <summary>
		/// new ontology concepts can be added while KEUI is running. we have to store the new data
		/// </summary>
		public void SaveAnnotationOntology(string fileName)
		{
			try
			{
				string RDFS = "http://www.w3.org/2000/01/rdf-schema#";
				string RDF = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";
				Entity rdfValue = RDF + "value";
				Entity rdfsComment = RDFS + "comment";

				// remove existing value properties
				_conceptsStore.Remove(new Statement(null, rdfValue, null));

				// create new value properties with updated values
				foreach (Statement s in _conceptsStore.Select(new Statement(null, rdfsComment, null)))
				{
					int value = GetConceptFrequencyCount(s.Subject.Uri);
					if (value > 0)
						_conceptsStore.Add(new Statement(s.Subject.Uri, rdfValue, (Literal)value.ToString()));
				}
				
				string tempName = fileName + ".temp";
				RdfXmlWriter.Options options = new RdfXmlWriter.Options();
				options.EmbedNamedNodes = false;
				using (RdfWriter writer = new RdfXmlWriter(tempName, options))
				{
					writer.Namespaces.AddNamespace("http://example.org/", "ex");
					writer.Namespaces.AddNamespace("http://www.w3.org/2002/27/owl#", "owl");
					writer.Namespaces.AddNamespace("http://dbpedia.org/ontology/", "DBpedia");
					writer.Namespaces.AddNamespace("http://ailab.ijs.si/alert/predicate/", "ailab");
					writer.Namespaces.AddNamespace("http://purl.org/dc/terms/", "purl");
					writer.Namespaces.AddNamespace("http://www.w3.org/1999/02/22-rdf-syntax-ns#", "rdf");
					writer.Namespaces.AddNamespace("http://www.w3.org/2000/01/rdf-schema#", "rdfs");

					writer.Write(_conceptsStore);
				}

				// if we successfully saved the ontology to the temp name only then rename the temp file to the correct name
				if (System.IO.File.Exists(fileName))
					System.IO.File.Delete(fileName);
				System.IO.File.Move(tempName, fileName);
			}
			catch (Exception ex)
			{
				AddEvent("Exception while saving the annotation ontology. Error: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while saving the annotation ontology", ex);
			}
		}

		public string GetAnnotationOntologyRDF(bool includeComment, bool includeLinksTo)
		{
			try
			{
				Store smallStore = new MemoryStore();
				foreach (Statement s in _conceptsStore.Select(new Statement(null, null, null)))
				{
					if (s.Predicate == RelRdfsComment && includeComment == false)
						continue;
					if (s.Predicate == RelDbAbstract && includeComment == false)
						continue;
					if (s.Predicate == RelLinksTo && includeLinksTo == false)
						continue;
					smallStore.Add(s);
				}

				System.IO.StringWriter strWriter = new System.IO.StringWriter();
				RdfXmlWriter.Options options = new RdfXmlWriter.Options();
				options.EmbedNamedNodes = false;
				using (RdfWriter writer = new RdfXmlWriter(strWriter, options))
				{
					writer.Namespaces.AddNamespace("http://example.org/", "ex");
					writer.Namespaces.AddNamespace("http://www.w3.org/2002/27/owl#", "owl");
					writer.Namespaces.AddNamespace("http://dbpedia.org/ontology/", "DBpedia");
					writer.Namespaces.AddNamespace("http://ailab.ijs.si/alert/predicate/", "ailab");
					writer.Namespaces.AddNamespace("http://purl.org/dc/terms/", "purl");
					writer.Namespaces.AddNamespace("http://www.w3.org/1999/02/22-rdf-syntax-ns#", "rdf");
					writer.Namespaces.AddNamespace("http://www.w3.org/2000/01/rdf-schema#", "rdfs");

					writer.Write(smallStore);
				}
				return strWriter.ToString();
			}
			catch (Exception ex)
			{
				AddEvent("Exception while sending the annotation ontology as string. Error: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while sending the annotation ontology as string", ex);
				return "";
			}
		}

		public bool StoreIsValid { get { return _conceptsStore != null; } }

		public SelectResult Select(Statement statement)
		{
			if (_conceptsStore == null)
				return null;
			return _conceptsStore.Select(statement);
		}

		#region add new data
		public void AddNewRDFData(string rdfData, bool removeExistingDataForSubjects)
		{
			MemoryStore store = new MemoryStore();
			RdfXmlReader reader = new RdfXmlReader(new System.IO.StringReader(rdfData));
			store.Import(reader);
			// its possible that we want to update info about some existing concept
			// if removeExistingDataForSubjects is true we first remove existing information before we start adding statements
			if (removeExistingDataForSubjects)
			{
				foreach (Entity subject in store.SelectSubjects(null, null))
					_conceptsStore.Remove(new Statement(subject, null, null));
			}
			foreach (Statement s in store.Select(new Statement(null, null, null)))
				AddNewStatement(s);
			foreach (Entity subject in store.SelectSubjects(null, null))
				UpdateSuggestionsForUri(subject.Uri);
		}

		public void AddNewStatement(Statement statement)
		{
			string subject = statement.Subject.Uri;
			string predicate = statement.Predicate.Uri;
			_conceptsStore.Add(statement);
			if (predicate == RelRdfsLabel)
			{
				string label  = ((Literal) statement.Object).Value;		
				string labelNormalized = Tokenization.NormalizeText(label);
				string labelLemmaPartsJoined = GetLemmatizedText(label);
				ConceptLabelToUri[labelNormalized] = subject;
				ConceptLabelNormalizedToUri[labelLemmaPartsJoined] = subject;
				if (!ConceptUriToLabels.ContainsKey(subject))
					ConceptUriToLabels[subject] = new List<string>();
				ConceptUriToLabels[subject].Add(label);
				//_conceptUriToFrequencyCount[s.Subject.Uri] = 0;
			}
			else if (predicate == RelRdfsComment)
				ConceptUriToDescription[subject] = ((Literal)statement.Object).Value;
			else if (predicate == RelDbAbstract)
				ConceptUriToDescription[subject] = ((Literal)statement.Object).Value;
			else if (predicate == RelRdfValue)
				_conceptUriToFrequencyCount[subject] = int.Parse(((Literal)statement.Object).Value);
		}
		#endregion

		#region concept frequency
		public void IncreaseConceptFrequencyCount(string conceptUri, int value = 1)
		{
			if (string.IsNullOrEmpty(conceptUri))
				return;
			_conceptUriToFrequencyCount[conceptUri] = _conceptUriToFrequencyCount.ContainsKey(conceptUri) ? _conceptUriToFrequencyCount[conceptUri] + value : value;
		}

		public void ClearFrequenciesForAllConcepts()
		{
			_conceptUriToFrequencyCount.Clear();
		}
		#endregion

		#region suggestion functionality
		private Dictionary<string, List<string>> _conceptPrefixToConceptUri = new Dictionary<string, List<string>>();
		private const int _prefixConceptSuggestionCount = 20;		// max number of concepts that can be suggested for a given prefix
		private const int _prefixConceptMaxSize = 6;				// max length of the prefix considered for suggestion
		
		public List<Tuple<string, string>> SuggestConceptsForPrefix(string prefix)
		{
			List<Tuple<string, string>> suggestions = new List<Tuple<string, string>>();
			try
			{
				if (string.IsNullOrEmpty(prefix))
					return suggestions;
				prefix = prefix.ToLower(System.Globalization.CultureInfo.InvariantCulture);
				prefix = prefix.Substring(0, Math.Min(prefix.Length, _prefixConceptMaxSize));
				if (_conceptPrefixToConceptUri.ContainsKey(prefix))
				{
					foreach (string label in _conceptPrefixToConceptUri[prefix])
					{
						string labelNormalized = Tokenization.NormalizeText(label);
						if (ConceptLabelToUri.ContainsKey(labelNormalized))
							suggestions.Add(new Tuple<string, string>(label, ConceptLabelToUri[labelNormalized]));
					}
				}
			}
			catch (Exception ex)
			{
				AddEvent("Exception while retrieving concept suggestions for prefix " + prefix + ". " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while retrieving concept suggestions for prefix " + prefix + ". ", ex);
			}
			return suggestions;
		}

		public void UpdateSuggestionsDict()
		{
			try
			{
				_conceptPrefixToConceptUri = new Dictionary<string, List<string>>();

				//AddEvent("Frequently annotated words:");
				//int i = 0;
				//foreach (string uri in (from uri in ConceptUriToDescription.Keys orderby GetConceptFrequencyCount(uri) descending select uri))
				//{
				//	i++;
				//	if (i > 20)
				//		break;
				//	AddEvent(GetConceptLabel(uri));
				//}

				// update the list of concepts suggestions - start from the most frequently annotated concepts
				foreach (string uri in (from uri in ConceptUriToDescription.Keys orderby GetConceptFrequencyCount(uri) descending select uri))
					UpdateSuggestionsForUri(uri);
			}
			catch (Exception ex)
			{
				AddEvent("Exception while updating suggestion dict for annotation ontology: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while updating suggestion dict for annotation ontology. ", ex);
			}
		}

		private void UpdateSuggestionsForUri(string uri)
		{
			HashSet<string> seenPrefixes = new HashSet<string>();
			List<string> labels = GetConceptLabels(uri);
			foreach (string label in labels)
			{
				for (int i = 1; i <= _prefixConceptMaxSize; i++)
				{
					// if label is too short then break
					if (label.Length < i) break;
					// normalize prefix
					string labelPrefix = label.Substring(0, i).ToLower();
					labelPrefix = Text.ReplaceUnicodeCharsWithAscii(labelPrefix);
					// check if the very similar label for this concept was already added to the list. if yes, then don't add it again.
					if (seenPrefixes.Contains(labelPrefix))
						continue;
					seenPrefixes.Add(labelPrefix);
					// create the suggestion list if not existing yet
					if (!_conceptPrefixToConceptUri.ContainsKey(labelPrefix))
						_conceptPrefixToConceptUri[labelPrefix] = new List<string>();
					// if we already have max number of suggestions for this prefix then do nothing
					if (_conceptPrefixToConceptUri[labelPrefix].Count > _prefixConceptSuggestionCount)
						continue;
					// add the label for the prefix. we could add uri but then we would not have all suggestions (e.g. form, forms)
					_conceptPrefixToConceptUri[labelPrefix].Add(label);
				}
			}
			//if (labels.Count == 0)
			//    AddEvent("Concept " + uri + " has no labels associated with it. Consider removing it");
		}
		#endregion

		#region helper functions
		private string GetLemmatizedText(string text)
		{
			string cleanName = Tokenization.NormalizeText(text);
			string[] parts = Tokenization.GetTextParts(cleanName);
			string[] lemmaParts = (from part in parts select _lemmaGen.GetLemma(part)).ToArray();
			return Tokenization.JoinStringParts(lemmaParts);
		}

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
