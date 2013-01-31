using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

//using Contextify;
//using Contextify.Base;
using System.Diagnostics;
using Contextify.Shared.Types;
using System.IO;
using System.Threading;
using System.Text.RegularExpressions;

using Apache.NMS;
using Apache.NMS.ActiveMQ;
using KEUIApp;
using Contextify.Shared.Base;
using GenLib.Text;
using SemWeb;

namespace ExtractProjectSpecificConcepts
{
	public partial class ProjectSpecificConcepts : Form
	{
		//MailData _mailData = null;
		private ListViewItem lvItem;
		private bool _computingCandidates = false;
		private bool _stopComputingCandidates = false;

		public ISession AQSession = null;
		//public ActiveMqHelper.TopicPublisher AQPublisherKEUI = null;
		public ActiveMqHelper.TopicSubscriber AQSubscriberKEUI = null;
		public static string PublisherName = "ConceptExtraction";
		public static string AQConsumerConceptExtractionId = PublisherName + ".subscriber";

		private Dictionary<int, TagInfoBase> _tagIdToTagInfo = new Dictionary<int, TagInfoBase>();
		private Dictionary<string, TagInfoBase> _tagIdStrToTagInfo = new Dictionary<string, TagInfoBase>();
		private Dictionary<int, List<TagInfoBase>> _tagIdToChildrenInfo = new Dictionary<int, List<TagInfoBase>>();
		private string _tagNameConcepts = "Annotation ontology";
		private string _tagNameCustomSources = "Custom sources";
		private string TopicNameKEUIRequest = "ALERT.Extractor.KEUIRequest";
		
		private Dictionary<string, string> _conceptIdToLabels = new Dictionary<string, string>();

		private Dictionary<string, string> _sequenceIdToType = new Dictionary<string, string>();
		private List<ListViewItem> _hiddenConceptItems = new List<ListViewItem>();
		private List<string> _conceptRelations = new List<string>() { "linksTo", "type", "sameAs", "isPartOf", "creator" };
		private Dictionary<string, string> _relationToUri = new Dictionary<string, string>();
		

		KEUIApp.ActiveMQSettings _activeMQSettings = null;
		string _fileNameActiveMQSettings = "ActiveMQSettings.xml";

		private Queue<int> _tagIdsToProcess = null;
		private int _tagIdsToProcessCount = 0;
		string _keywordsForTagMethod = null;
		int _keywordsForTagCount = 1;
		bool _editingExistingConcept = false;

		public ProjectSpecificConcepts()
		{
			InitializeComponent();
			_relationToUri["linksTo"] = AnnotationOntology.RelLinksTo.Uri;
			_relationToUri["type"] = AnnotationOntology.RelRdfType.Uri;
			_relationToUri["sameAs"] = AnnotationOntology.RelOwlSameAs.Uri;
			_relationToUri["isPartOf"] = AnnotationOntology.RelPurlIsPartOf.Uri;
			_relationToUri["creator"] = AnnotationOntology.RelPurlCreator.Uri;
			
			this.Load += new EventHandler(ProjectSpecificConcepts_Load);
			this.FormClosed += new FormClosedEventHandler(ProjectSpecificConcepts_FormClosed);

			ListViewRelated.MouseUp += new MouseEventHandler(ListViewRelated_MouseUp);
			ListViewRelated.ItemCheck += new ItemCheckEventHandler(ListViewRelated_ItemCheck);
			
			ListViewCandidates.ItemCheck += new ItemCheckEventHandler(ListViewCandidates_ItemCheck);
			ListViewCandidates.ListViewItemSorter = new CandidateItemComparer();
			ListViewCandidates.Sorting = SortOrder.Descending;

			PopulateComboBoxRelations();
			UpdateGuiState();

			String AppName = "KEUIApp";
			String IndexingServerFolder = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), AppName);
			_activeMQSettings = new ActiveMQSettings(Path.Combine(IndexingServerFolder, _fileNameActiveMQSettings));
		}

		void ProjectSpecificConcepts_Load(object sender, EventArgs e)
		{
			try
			{
				IConnectionFactory factory = new ConnectionFactory(_activeMQSettings.BrokerUri);
				IConnection connection = factory.CreateConnection();
				connection.Start();
				AQSession = connection.CreateSession();

				//AQPublisherKEUI = new ActiveMqHelper.TopicPublisher(AQSession, Defaults.TopicNameKEUIRequest);

				AQSubscriberKEUI = new ActiveMqHelper.TopicSubscriber(AQSession, _activeMQSettings.TopicNameKEUIPublishResponse);
				AQSubscriberKEUI.Start(AQConsumerConceptExtractionId);
				AQSubscriberKEUI.OnMessageReceived += new ActiveMqHelper.TopicSubscriber.MessageReceivedDelegate(KEUIResponse_OnMessageReceived);

				LoadTags();
				SendRequestForTheAnnotationOntology(); 
			}
			catch (Exception ex)
			{
				MessageBox.Show("Error while loading: " + ex.Message);
			}
		}

		private void LoadTags()
		{
			// load tags
			string newSequenceId;
			int startIndex = 0;
			int count = 1000;
			string request = Defaults.BuildRequest(PublisherName, "GetTagInfo", String.Format("<params startIndex=\"{0}\" count=\"{1}\" />", startIndex, count), out newSequenceId);
			StatusText.Text = "Loading tags...";
			SendRequest(request, newSequenceId, "GetTagInfo");
		}
		

		// dispose the async web getter
		void ProjectSpecificConcepts_FormClosed(object sender, FormClosedEventArgs e)
		{
			if (AQSubscriberKEUI != null) AQSubscriberKEUI.Dispose();
			AQSubscriberKEUI = null;
		}

		void KEUIResponse_OnMessageReceived(string message)
		{
			try
			{
				HtmlAgilityPack.XmlDocument eventDoc = new HtmlAgilityPack.XmlDocument();
				eventDoc.LoadXml(message);
				var sequenceIdNode = eventDoc.DocumentNode.SelectSingleNode("//ns1:head/ns1:sequencenumber");
				string sequenceId = sequenceIdNode.InnerText;
				Trace.WriteLine("Recieved response to " + sequenceId);
				var responseNode = eventDoc.DocumentNode.SelectSingleNode("//ns1:eventData/s1:keuiResponse");
				string responseData = responseNode.InnerHtml;
				var requestNode = eventDoc.DocumentNode.SelectSingleNode("//ns1:eventData/s1:keuiRequest");
				var requestTypeNode = eventDoc.DocumentNode.SelectSingleNode("//ns1:eventData/s1:keuiRequest/s1:requestType");
				var requestDataNode = eventDoc.DocumentNode.SelectSingleNode("//ns1:eventData/s1:keuiRequest/s1:requestData");
				string requestType = requestTypeNode.InnerText;

				if (requestType == "GetTagInfo")
				{
					var paramNode = requestDataNode.SelectSingleNode("./params");
					int startIndex = paramNode.GetAttributeValue("startIndex", 0);
					int count = paramNode.GetAttributeValue("count", 1000);

					HtmlAgilityPack.HtmlNodeCollection tags = responseNode.SelectNodes("./TagInfoBase") ?? new HtmlAgilityPack.HtmlNodeCollection(null);
					foreach (var tag in tags)
					{
						string tagData = tag.OuterHtml;
						TagInfoBase newTag = TagInfoBase.FromXML(tagData);
						_tagIdToTagInfo[newTag.TagId] = newTag;
						if (!string.IsNullOrEmpty(newTag.TagIdStr))
							_tagIdStrToTagInfo[newTag.TagIdStr] = newTag;
						if (!_tagIdToChildrenInfo.ContainsKey(newTag.ParentTagId))
							_tagIdToChildrenInfo[newTag.ParentTagId] = new List<TagInfoBase>();
						_tagIdToChildrenInfo[newTag.ParentTagId].Add(newTag);
					}

					if (tags.Count() == count)
					{
						string newSequenceId;
						string request = Defaults.BuildRequest(PublisherName, "GetTagInfo", String.Format("<params startIndex=\"{0}\" count=\"{1}\" />", startIndex + count, count), out newSequenceId);
						//this.BeginInvoke((Action)(() => { SendRequest(request, newSequenceId, "GetTagInfo"); }));
						SendRequest(request, newSequenceId, "GetTagInfo");
					}
					Invoke((Action)(() =>
						{
							if (tags.Count == count)
								StatusText.Text = String.Format("Loading tags... ({0} tags loaded)", _tagIdToTagInfo.Count);
							else
								StatusText.Text = "Tags successfully loaded.";
						}));

				}
				else if (requestType == "GetAnnotationOntologyRDF")
				{
					LoadOntology(responseData);
				}
				else if (requestType == "GetSimilarConcepts")
				{
					foreach (HtmlAgilityPack.HtmlNode conceptNode in responseNode.SelectNodes(".//item") ?? new HtmlAgilityPack.HtmlNodeCollection(null))
					{
						string uri = conceptNode.GetAttributeValue("uri", "");
						string label = conceptNode.GetAttributeValue("label", "");
						AddRelatedConcept(label, uri, _conceptRelations[0]);
					}
				}
				else if (requestType == "ExecuteCommand")
					Response_ExecuteCommand(responseData, sequenceId, message);
				else if (requestType == "SetData")
					Response_SetData(responseData, sequenceId, message);
				else if (requestType == "Query")
					Response_Query(responseData, sequenceId, message);
				else
					MessageBox.Show("Don't know how to process request type " + requestType + ".");

				// remove the id of the processed event
				if (_sequenceIdToType.ContainsKey(sequenceId))
					_sequenceIdToType.Remove(sequenceId);
			}
			catch (Exception ex)
			{
				Trace.WriteLine("Exception while processing response: " + ex.Message);
			}
		}

		void AddRelatedConcept(string label, string conceptUri, string relation)
		{
			Invoke((Action)(() =>
			{
				ListViewItem item = new ListViewItem(new string[] { "", string.Format("{0} ({1})", label, conceptUri), relation });
				item.Tag = conceptUri;
				ListViewRelated.Items.Add(item);
			}));
		}

		void SendRDFData(string rdfData)
		{
			try
			{
				rdfData = rdfData.TrimStart("<?xml version=\"1.0\"?>");
				rdfData = rdfData.Trim();
				Trace.WriteLine("Sending rdf data");
				string eventData = Defaults.BuildNewRDFData(rdfData);
				lock (_sendingLock)
				{
					using (var publisher = new ActiveMqHelper.TopicPublisher(AQSession, _activeMQSettings.TopicNameAnnotexPublishConceptNew))
					{
						publisher.SendMessage(eventData);
					}
				}
			}
			catch (Exception ex)
			{
				Trace.WriteLine("Exception while posting a request: " + ex.Message);
			}
			
		}

		private object _sendingLock = new object();
		void SendRequest(string request, string requestId, string requestType)
		{
			try
			{
				Trace.WriteLine("Sending request with id " + requestId);
				lock (_sendingLock)
				{
					if (requestType != null)
						_sequenceIdToType[requestId] = requestType;
					using (var publisher = new ActiveMqHelper.TopicPublisher(AQSession, TopicNameKEUIRequest))
					{
						publisher.SendMessage(request);
					}
				}
			}
			catch (Exception ex)
			{
				Trace.WriteLine("Exception while posting a request: " + ex.Message);
			}
		}

		void Response_Query(string response, string sequenceId, string wholeEvent)
		{
			if (!_sequenceIdToType.ContainsKey(sequenceId))
				return;

			if (_sequenceIdToType[sequenceId] == "keywordsForTagId")
			{
				List<Tuple<string, double>> keywords = ParseKeywords(response);
				foreach (var kw in keywords)
					AddCandidateToListView(kw);
				RequestKeywordsForNextTagId();
			}
			else if (_sequenceIdToType[sequenceId] == "keywordsForDescription")
			{
				List<Tuple<string, double>> keywords = ParseKeywords(response);
				string kws = string.Join(" ", from kw in keywords select kw.Item1);
				Invoke((Action)(() => { TextBoxDescription.Text = kws; }));
			}
			else if (_sequenceIdToType[sequenceId] == "keywordsForRecentPosts")
			{
				List<Tuple<string, double>> keywords = ParseKeywords(response);
				foreach (var kw in keywords)
					AddCandidateToListView(kw);
				Invoke((Action)(() => { StatusText.Text = ""; })); 
			}
			else if ((new List<string>() { "kmeans", "hkmeans" }).Contains(_sequenceIdToType[sequenceId])) {
				List<Tuple<string, double>> keywords = ParseKeywords(response);
				foreach (var kw in keywords)
					AddCandidateToListView(kw);
				Invoke((Action)(() => { StatusText.Text = ""; }));
			}
			else if (_sequenceIdToType[sequenceId] == "nGram") {
				List<Tuple<string, double>> phrases = ParseNGrams(response, true);
				foreach (var kw in phrases)
					AddCandidateToListView(kw);
				Invoke((Action)(() => { StatusText.Text = ""; }));
			}
			else if (_sequenceIdToType[sequenceId] == "keywordsUsingNeutralSources") {
				List<Tuple<string, double>> keywords = ParseKeywords(response);
				foreach (var kw in keywords)
					AddCandidateToListView(kw);
				Invoke((Action)(() => { StatusText.Text = ""; }));
			}
		}

		void Response_ExecuteCommand(string response, string sequenceId, string wholeEvent)
		{
			Debug.Assert(false, "No commands should be issued. Check who needs this response: " + response);
		}

		void Response_SetData(string response, string sequenceId, string wholeEvent)
		{
			new NotImplementedException("SetData handler is not written yet");
		}

		void RequestKeywordsForNextTagId()
		{
			if (_stopComputingCandidates || _tagIdsToProcess.Count == 0)
			{
				_computingCandidates = false;
				_stopComputingCandidates = false;
				Invoke((Action)(() =>
				{
					StatusText.Text = "";
					StatusProgressBar.Value = 0;
				}));
				return;
			}
			int tagsLeft = _tagIdsToProcess.Count;
			int tagId = _tagIdsToProcess.Dequeue();
			GeneralQuery query = new GeneralQuery(new QGeneralParams() { ResultData = QResultData.keywordData, KeywordCount = _keywordsForTagCount, SampleSize = -1, KeywordMethod = GetKeywordMethod(_keywordsForTagMethod) },
				new QArgs(new QTagIdCond(tagId)));
			string requestId;
			string request = Defaults.BuildRequest(PublisherName, "Query", query.ToString(), out requestId);
			SendRequest(request, requestId, "keywordsForTagId");
			Invoke((Action)(() => {
				string tagName = _tagIdToTagInfo[tagId].TagName;
				StatusText.Text = String.Format("Computing concepts for tag {0} with id = {1} ({2:F2}% done) ", tagName, tagId, 100 * (_tagIdsToProcessCount - tagsLeft) / (float)_tagIdsToProcessCount);
				StatusProgressBar.Value = _tagIdsToProcessCount - tagsLeft; 
			}));
		}

		#region ListViewCandiates and the related functionality
		void ListViewCandidates_ItemCheck(object sender, ItemCheckEventArgs e)
		{
			int itemIndex = e.Index;
			ListViewCandidates.Items.RemoveAt(0);	
		}

		private void ButtonGetCandidates_Click(object sender, EventArgs e)
		{
			ButtonGetCandidates.ContextMenuStrip.Show(ButtonGetCandidates, 0, ButtonGetCandidates.Height);
		}
		
		public void AddCandidateToListView(Tuple<string, double> kw)
		{
			// ignore suggestions that are already present in the ontology
			if (_ao.ConceptLabelToUri.ContainsKey(kw.Item1.ToLower()))
				return;

			Invoke((Action)(() =>
			{
				int i = 0;
				for (i = 0; i < ListViewCandidates.Items.Count; i++)
				{
					Tuple<string, double> tag = ListViewCandidates.Items[i].Tag as Tuple<string, double>;
					if (tag.Item1 == kw.Item1)
					{
						if (tag.Item2 < kw.Item2) { ListViewCandidates.Items.RemoveAt(i); break; }		// the existing item wgt is smaller, remove it
						else return;																	// the existing item has higher wgt - we return
					}
				}
				//i = 0;
				//for (i = 0; i < ListViewCandidates.Items.Count; i++)
				//{
				//    if ((ListViewCandidates.Items[i].Tag as Tuple<string, double>).Item2 < kw.Item2)
				//        break;
				//}
				//ListViewCandidates.Items.Insert(i, new ListViewItem(String.Format("{0} ({1:F4})", kw.Item1, kw.Item2)) { Tag = kw });
				ListViewCandidates.Items.Add(new ListViewItem(String.Format("{0} ({1:F4})", kw.Item1, kw.Item2)) { Tag = kw });
			}));
		}

		private void ButtonLoad_Click(object sender, EventArgs e)
		{
			OpenFileDialog dlg = new OpenFileDialog();
			dlg.InitialDirectory = Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);
			dlg.Filter = "Concept files (*.cnc)|*.cnc|All files (*.*)|*.*";
			if (dlg.ShowDialog() == System.Windows.Forms.DialogResult.OK)
			{
				ListViewCandidates.Items.Clear();
				ListViewCandidates.Sorting = SortOrder.None;
				ListViewCandidates.BeginUpdate();
				if (File.Exists(dlg.FileName))
				{
					foreach (string line in File.ReadAllLines(dlg.FileName))
					{
						Match m = Regex.Match(line, "\"(?<name>[^\"]+)\",.*?(?<wgt>[.0-9]+)");
						if (m.Success)
						{
							string name = m.Groups["name"].Value;
							double wgt = double.Parse(m.Groups["wgt"].Value, System.Globalization.NumberFormatInfo.InvariantInfo);
							if (_ao.ConceptLabelToUri.ContainsKey(name.ToLower()))
								continue;
							ListViewCandidates.Items.Add(new ListViewItem(String.Format("{0} ({1})", name, wgt.ToString("F4", System.Globalization.NumberFormatInfo.InvariantInfo))) { Tag = new Tuple<string, double>(name, wgt) });
						}
					}
				}
				ListViewCandidates.EndUpdate();
				ListViewCandidates.ListViewItemSorter = new CandidateItemComparer();
				ListViewCandidates.Sorting = SortOrder.Descending;
			}
			UpdateGuiState();
		}

		private void ButtonSave_Click(object sender, EventArgs e)
		{
			SaveFileDialog dlg = new SaveFileDialog();
			dlg.InitialDirectory = Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);
			dlg.Filter = "Concept files (*.cnc)|*.cnc|All files (*.*)|*.*";
			if (dlg.ShowDialog() == System.Windows.Forms.DialogResult.OK)
			{
				StringBuilder output = new StringBuilder();
				foreach (ListViewItem item in ListViewCandidates.Items)
				{
					Tuple<string, double> kw = item.Tag as Tuple<string, double>;
					output.Append(String.Format("\"{0}\", {1}", kw.Item1, kw.Item2.ToString("F4", System.Globalization.NumberFormatInfo.InvariantInfo) + Environment.NewLine));
				}
				File.WriteAllText(dlg.FileName, output.ToString());
			}
		}


		private void ButtonClear_Click(object sender, EventArgs e)
		{
			int candidatesCount = ListViewCandidates.Items.Count;
			if (candidatesCount == 0)
				return;
			if (MessageBox.Show("Are you sure you want to remove all the candidates from the list?", "Confirmation", MessageBoxButtons.YesNo, MessageBoxIcon.Question) == System.Windows.Forms.DialogResult.Yes)
			{
				ListViewCandidates.Items.Clear();
			}
		}

		private void ignoreSelectedCandidatesToolStripMenuItem_Click(object sender, EventArgs e)
		{
			List<ListViewItem> selected = new List<ListViewItem>();
			foreach (ListViewItem item in ListViewCandidates.SelectedItems)
				selected.Add(item);
			foreach (ListViewItem item in selected)
				ListViewCandidates.Items.Remove(item);
		}

		private void ListViewCandidates_MouseClick(object sender, MouseEventArgs e)
		{
			if (e.Button == System.Windows.Forms.MouseButtons.Right && ListViewCandidates.SelectedItems.Count > 0)
				contextMenuStrip1.Show(ListViewCandidates, e.Location);
		}
		#endregion

		#region ListViewRelated and related functionality
		private void PopulateComboBoxRelations()
		{
			// load possible relations between the concepts
			ComboBoxRelations.Items.Clear();
			foreach (string relation in _conceptRelations)
				ComboBoxRelations.Items.Add(relation);

			ComboBoxRelations.SelectedValueChanged += new EventHandler(ComboBoxRelations_SelectedValueChanged);
			ComboBoxRelations.Leave += new EventHandler(ComboBoxRelations_Leave);
			ComboBoxRelations.KeyPress += new KeyPressEventHandler(ComboBoxRelations_KeyPress);
		}

		// remove the checked item
		void ListViewRelated_ItemCheck(object sender, ItemCheckEventArgs e)
		{
			int itemIndex = e.Index;
			ListViewRelated.Items.RemoveAt(itemIndex);
			if (ComboBoxRelations.Visible)
				ComboBoxRelations.SendToBack();
		}

		// lost the focus on the item
		void ComboBoxRelations_Leave(object sender, EventArgs e)
		{
			if (!string.IsNullOrEmpty(ComboBoxRelations.Text))
				lvItem.SubItems[2].Text = ComboBoxRelations.Text;		// Set text of ListView item to match the ComboBox.
			ComboBoxRelations.Visible = false;		// Hide the ComboBox.
		}

		// list view item was clicked. show the combobox
		void ListViewRelated_MouseUp(object sender, MouseEventArgs e)
		{
			// Get the item on the row that is clicked.
			lvItem = ListViewRelated.GetItemAt(e.X, e.Y);

			// Make sure that an item is clicked.
			if (lvItem != null)
			{
				Rectangle ClickedItem = lvItem.SubItems[2].Bounds;
				if (ClickedItem.Left < 0)
				{
					ClickedItem.Width += ClickedItem.Left;		// reduce the width of the combo by the amount that is not visible
					ClickedItem.X = 0;
				}
				if (ClickedItem.Right > ListViewRelated.Width)
					ClickedItem.Width = ListViewRelated.Width - ClickedItem.X - 2;	
				ClickedItem.X += ListViewRelated.Left + 2;
				ClickedItem.Y += ListViewRelated.Top;
					
				// Assign calculated bounds to the ComboBox.
				ComboBoxRelations.Bounds = ClickedItem;

				// Set default text for ComboBox to match the item that is clicked.
				ComboBoxRelations.Text = lvItem.SubItems[2].Text;

				// Display the ComboBox, and make sure that it is on top with focus.
				ComboBoxRelations.Visible = true;
				ComboBoxRelations.BringToFront();
				ComboBoxRelations.Focus();
			}
		}

		void ComboBoxRelations_KeyPress(object sender, KeyPressEventArgs e)
		{
			if (e.KeyChar == (char)(int)Keys.Escape)
			{
				ComboBoxRelations.Text = lvItem.SubItems[2].Text;	// Reset the original text value, and then hide the ComboBox.
				ComboBoxRelations.Visible = false;
			}
			else if (e.KeyChar == (char)(int)Keys.Enter)
				ComboBoxRelations.Visible = false;	// Hide the ComboBox.
		}

		void ComboBoxRelations_SelectedValueChanged(object sender, EventArgs e)
		{
			if (lvItem != null)
				lvItem.SubItems[2].Text = ComboBoxRelations.Text;
			ComboBoxRelations.Visible = false;
		}

		private void ButtonAutoAddRelatedConcepts_Click(object sender, EventArgs e)
		{
			ListViewRelated.Items.Clear();

			if (string.IsNullOrEmpty(TextBoxDescription.Text))
			{
				MessageBox.Show("In order to automatically identify related concepts you first have to provide the concept description.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Information);
				return;
			}
			if (GetTag(-1, _tagNameConcepts) == null)
			{
				MessageBox.Show("The tag for the Annotation ontology concepts was not identified. Unable to automatically suggest related concepts.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Information);
				return;
			}
			
			string requestId = "";
			CustomQuery query = new CustomQuery(new QSimilarItemsParam() { TextToCompare = TextBoxDescription.Text, MaxCount = 10 }, new QArgs(new QTagIdCond(GetTag(-1, _tagNameConcepts).TagId)));
			string request = Defaults.BuildRequest(PublisherName, "GetSimilarConcepts", query.ToString(), out requestId);
			SendRequest(request, requestId, "GetSimilarConcepts");			
		}

		// choose an existing concept from the list and 
		private void ButtonManuallyAddRelatedConcepts_Click(object sender, EventArgs e)
		{
			SelectExistingConcept dialog = new SelectExistingConcept(_ao, true);
			if (dialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
			{
				foreach (string uri in dialog.SelectedConceptUris)
				{
					string label = _ao.GetConceptLabel(uri);
					AddRelatedConcept(label, uri, _conceptRelations[0]);
				}
			}
		}
		#endregion

		private QKeywordMethod GetKeywordMethod(string method)
		{
			if (method == "SVM") return QKeywordMethod.SVM;
			else if (method == "globalConceptSpV") return QKeywordMethod.globalConceptSpV;
			else if (method == "localConceptSpV") return QKeywordMethod.localConceptSpV;
			return QKeywordMethod.SVM;
		}

		private void extractPhrasesToolStripMenuItem_Click(object sender, EventArgs e)
		{
			string requestId;
			CandidatesPhrases candidateSettings = new CandidatesPhrases();
			if (candidateSettings.ShowDialog() == System.Windows.Forms.DialogResult.OK)
			{
				int count = int.Parse(candidateSettings.TextBoxCandidateCount.Text);
				int mnNGramFq = int.Parse(candidateSettings.TextBoxMnFq.Text);

				CustomQuery query = new CustomQuery(new QNGramsParam() { MnNGramFq = mnNGramFq, MnNGramLen = 2, MxNGramCount = count });
				string request = Defaults.BuildRequest(PublisherName, "Query", query.ToString(), out requestId);

				StatusText.Text = "Computing new candidates...";
				SendRequest(request, requestId, "nGram");
			}
		}

		private void extractCandidatesFromRecentPostsToolStripMenuItem_Click(object sender, EventArgs e)
		{
			string requestId;
			CandidatesRecent candidateSettings = new CandidatesRecent();
			if (candidateSettings.ShowDialog() == System.Windows.Forms.DialogResult.OK)
			{
				int count = int.Parse(candidateSettings.TextBoxCandidateCount.Text);
				string method = candidateSettings.GetConceptExtractionMethod();

				GeneralQuery query = new GeneralQuery(new QGeneralParams() { ResultData = QResultData.keywordData, KeywordMethod = GetKeywordMethod(method), KeywordCount = count }, new QArgs(new QTimelineCond(candidateSettings.DateTimeStart.Value, candidateSettings.DateTimeEnd.Value)));
				string request = Defaults.BuildRequest(PublisherName, "Query", query.ToString(), out requestId);
				
				StatusText.Text = "Computing new candidates...";
				SendRequest(request, requestId, "keywordsForRecentPosts");
			}
		}
		
		private void extractCandidatesUsingClusteringToolStripMenuItem_Click(object sender, EventArgs e)
		{
			try
			{
				if (_computingCandidates)
				{
					ButtonGetCandidates.Text = "Please wait...";
					_stopComputingCandidates = true;
					return;
				}

				// these items should be ignored from computing kmeans or hkmeans
				HashSet<int> ignoredTagsHash = new HashSet<int> { GetTagId(-1, _tagNameConcepts), GetTagId(-1, _tagNameCustomSources) };
				ignoredTagsHash.Remove(TagInfoBase.InvalidTagId);
				QArgs args = null;
				if (ignoredTagsHash.Count > 0)
					args = new QArgs(null, new[] { new QTagIdCond(ignoredTagsHash) });

				string requestId;
				CandidatesClusters candidateSettings = new CandidatesClusters(this);
				if (candidateSettings.ShowDialog() == System.Windows.Forms.DialogResult.OK)
				{
					ListViewCandidates.Items.Clear();
					int count = int.Parse(candidateSettings.TextBoxCandidateCount.Text);
					StatusText.Text = "Computing new candidates...";
					QKeywordMethod method = candidateSettings.GetConceptExtractionMethod();
					if (candidateSettings.RadioTags.Checked)
					{
						StatusProgressBar.Minimum = 0;
						StatusProgressBar.Maximum = candidateSettings.SelectedTags.Count;
						_tagIdsToProcess = new Queue<int>(candidateSettings.SelectedTags);
						_tagIdsToProcessCount = candidateSettings.SelectedTags.Count;
						_keywordsForTagMethod = method.ToString();
						_keywordsForTagCount = candidateSettings.GetConceptCount();
						_computingCandidates = true;
						RequestKeywordsForNextTagId();
					}
					else if (candidateSettings.RadioKMeans.Checked)
					{
						int k = int.Parse(candidateSettings.TextBoxKMeans.Text);
						CustomQuery query = new CustomQuery(new QKwdKMeansParam() { K = k, KeywordCount = count, KeywordMethod = method }, args);
						string request = Defaults.BuildRequest(PublisherName, "Query", query.ToString(), out requestId);
						SendRequest(request, requestId, "kmeans");
					}
					else if (candidateSettings.RadioHKMeans.Checked)
					{
						int min = int.Parse(candidateSettings.TextBoxHKMeansMin.Text);
						int max = int.Parse(candidateSettings.TextBoxHKMeansMax.Text);
						CustomQuery query = new CustomQuery(new QKwdHKMeansParam() { MnDocsPerCluster = min, MxDocsPerCluster = max, KeywordCount = count, KeywordMethod = method }, args);
						string request = Defaults.BuildRequest(PublisherName, "Query", query.ToString(), out requestId);
						SendRequest(request, requestId, "hkmeans");
					}
				}
			}
			catch (Exception ex)
			{
				MessageBox.Show("Exception while computing candidates: " + ex.Message);
			}
		}

		private void extractCandidatesUsingNeutralContentMenuItem_Click(object sender, EventArgs e)
		{
			string requestId;
			CandidatesUsingSVM candidateSettings = new CandidatesUsingSVM(this);
			if (candidateSettings.ShowDialog() == System.Windows.Forms.DialogResult.OK)
			{
				int keywordCount = candidateSettings.GetConceptCount();
				int timeLimit = candidateSettings.GetTimeLimit();
				//string positiveExamples = "<tagIds>" + String.Join(",", candidateSettings.ProjectTags) + "</tagIds>";
				//string negativeExamples = "<tagIds>" + String.Join(",", candidateSettings.NeutralTags) + "</tagIds>";
				CustomQuery query = new CustomQuery(new QKwdSVMParam() { KeywordCount = keywordCount, TimeLimit = timeLimit },
					new QArgs(new QTagIdCond(candidateSettings.ProjectTags)),
					new QArgsNegative(new QTagIdCond(candidateSettings.NeutralTags)));
				
				string request = Defaults.BuildRequest(PublisherName, "Query", query.ToString(), out requestId);

				StatusText.Text = "Computing new candidates...";
				SendRequest(request, requestId, "keywordsUsingNeutralSources");
			}
		}	

		private void ButtonLearnDescription_Click(object sender, EventArgs e)
		{
			string conceptLabels = TextBoxLabels.Text;
			if (string.IsNullOrEmpty(conceptLabels))
			{
				MessageBox.Show("In order to find keywords for a concept you first have to enter a set of labels for the concept.");
				return;
			}
			GeneralQuery query = new GeneralQuery(new QGeneralParams() { ResultData = QResultData.keywordData }, new QArgs(new QKeywordCond(conceptLabels)));
			string requestId;
			string request = Defaults.BuildRequest(PublisherName, "Query", query.ToString(), out requestId);
			SendRequest(request, requestId, "keywordsForDescription");
		}

		private void ListViewCandidates_SelectedIndexChanged(object sender, EventArgs e)
		{
			_editingExistingConcept = false;
			TextBoxLabels.Text = "";
			if (ListViewCandidates.SelectedItems.Count == 1 && (ListViewCandidates.SelectedItems[0].Tag as Tuple<string, double>) != null)
				TextBoxLabels.Text = (ListViewCandidates.SelectedItems[0].Tag as Tuple<string, double>).Item1;
			TextBoxDescription.Text = "";
			TextBoxConceptUri.Text = "";
			ListViewRelated.Items.Clear();
			UpdateGuiState();
		}

		private void TextBoxLabels_TextChanged(object sender, EventArgs e)
		{
			UpdateGuiState();
		}

		private void TextBoxDescription_TextChanged(object sender, EventArgs e)
		{
			UpdateGuiState();
		}

		private void ButtonEditExisting_Click(object sender, EventArgs e)
		{
			if (!_ao.StoreIsValid)
			{
				MessageBox.Show("The annotation ontology is not loaded yet. Please wait.");
				return;
			}
			SelectExistingConcept dialog = new SelectExistingConcept(_ao, false);
			if (dialog.ShowDialog() == System.Windows.Forms.DialogResult.OK && dialog.SelectedConceptUris.Count == 1)
			{
				string conceptUri = dialog.SelectedConceptUris[0];
				TextBoxDescription.Text = _ao.GetConceptDescription(conceptUri);
				TextBoxConceptUri.Text = conceptUri;
				TextBoxLabels.Text = string.Join(", ", _ao.GetConceptLabels(conceptUri));
				foreach (string key in _relationToUri.Keys)
				{
					string relationUri = _relationToUri[key];
					foreach (Statement s in _ao.Select(new Statement(new Entity(conceptUri), (Entity)relationUri, null)))
					{
						string objectUri = s.Object.Uri;
						if (string.IsNullOrEmpty(objectUri))
							objectUri = ((Literal)s.Object).Value;
						string objectLabel = _ao.GetConceptLabel(objectUri);
						if (!string.IsNullOrEmpty(objectLabel))
							AddRelatedConcept(objectLabel, objectUri, key);
					}
				}
				_editingExistingConcept = true;
			}
		}

		#region add concept
		private void ButtonAddSelectedConcept_Click(object sender, EventArgs e)
		{
			string candidate = TextBoxLabels.Text;
			Debug.Assert(!string.IsNullOrEmpty(candidate));

			if (string.IsNullOrEmpty(TextBoxConceptUri.Text))
			{
				MessageBox.Show("You have to specify a URI of the concept.");
				return;
			}

			if (_ao.ConceptUriToDescription.ContainsKey(TextBoxConceptUri.Text) && _editingExistingConcept == false)
			{
				MessageBox.Show("The concept with uri " + TextBoxConceptUri.Text + " already exists. Please specify a unique URI.");
				return;
			}

			// compute what are all the labels for the concept
			List<string> labels = candidate.Split(new char[] { ',' }, StringSplitOptions.RemoveEmptyEntries).ToList();
			labels = (from label in labels select label.Trim()).ToList();

			if (labels.Count == 0)
			{
				MessageBox.Show("You have to specify at least one label for the concept.");
				return;
			}

			string uri = TextBoxConceptUri.Text;
			Entity concept = new Entity(uri);
			
			Store tempStore = new MemoryStore();
			foreach (string label in labels)
			{
				if (_ao.ConceptLabelToUri.ContainsKey(label.ToLower()) && _editingExistingConcept == false)
				{
					MessageBox.Show("The label " + label + " is already assigned to concept " + _ao.ConceptLabelToUri[label.ToLower()] + ". Skipping the label.");
					continue;
				}
				tempStore.Add(new Statement(concept, AnnotationOntology.RelRdfsLabel, (Literal)label));
			}

			if (!string.IsNullOrEmpty(TextBoxDescription.Text))
				tempStore.Add(new Statement(concept, AnnotationOntology.RelRdfsComment, (Literal)TextBoxDescription.Text));

			foreach (ListViewItem item in ListViewRelated.Items)
			{
				string conceptUri = (string)item.Tag;
				string relationStr = item.SubItems[2].Text;
				tempStore.Add(new Statement(concept, (Entity)_relationToUri[relationStr], new Entity(conceptUri)));
			}

			// send the statements as an event
			System.IO.StringWriter strWriter = new System.IO.StringWriter();
			RdfXmlWriter.Options options = new RdfXmlWriter.Options();
			options.EmbedNamedNodes = false;
			using (RdfWriter writer = new RdfXmlWriter(strWriter, options))
				writer.Write(tempStore);
			string newRDFData = strWriter.ToString();

			SendRDFData(newRDFData);
			_ao.AddNewRDFData(newRDFData, true);

			// remove the added items
			List<ListViewItem> selectedItems = new List<ListViewItem>(from ListViewItem item in ListViewCandidates.SelectedItems select item);
			foreach (var item in selectedItems)
				ListViewCandidates.Items.Remove(item);
			TextBoxLabels.Text = "";
			TextBoxConceptUri.Text = "";
			TextBoxDescription.Text = "";
			ListViewRelated.Items.Clear();
		}

		private void ButtonGroupAdd_Click(object sender, EventArgs e)
		{
			if (ListViewCandidates.SelectedItems.Count < 2)
			{
				MessageBox.Show("To use the Group add functionality you have to select at least 2 candidates from the list");
				return;
			}

			Store tempStore = new MemoryStore();
			foreach (ListViewItem item in ListViewCandidates.SelectedItems)
			{
				string label = (item.Tag as Tuple<string, double>).Item1;
				string uri = GetSuggestedConceptUri(label);
				if (_ao.ConceptUriToDescription.ContainsKey(uri))
				{
					MessageBox.Show("The concept with uri " + uri + " already exists. Ignoring it.");
					continue;
				}
				if (_ao.ConceptLabelToUri.ContainsKey(label.ToLower()))
				{
					MessageBox.Show("The label " + label + " is already assigned to concept " + _ao.ConceptLabelToUri[label.ToLower()] + ". Skipping the label.");
					continue;
				}
				tempStore.Add(new Statement(new Entity(uri), AnnotationOntology.RelRdfsLabel, (Literal)label));
			}

			// remove the added items
			List<ListViewItem> selectedItems = new List<ListViewItem>(from ListViewItem item in ListViewCandidates.SelectedItems select item);
			foreach (var item in selectedItems)
				ListViewCandidates.Items.Remove(item);

			// send the statements as an event
			System.IO.StringWriter strWriter = new System.IO.StringWriter();
			RdfXmlWriter.Options options = new RdfXmlWriter.Options();
			options.EmbedNamedNodes = false;
			using (RdfWriter writer = new RdfXmlWriter(strWriter, options))
				writer.Write(tempStore);
			string newRDFData = strWriter.ToString();
			SendRDFData(newRDFData);
			_ao.AddNewRDFData(newRDFData, true);
		}
		#endregion

		string _prefix = "http://ailab.ijs.si/alert/resource/";
		private void ButtonSuggestConceptURI_Click(object sender, EventArgs e)
		{
			string candidate = null;
			if (!string.IsNullOrEmpty(TextBoxLabels.Text))
			{
				candidate = TextBoxLabels.Text;
				if (candidate.Contains(","))
					candidate = candidate.Substring(0, candidate.IndexOf(','));
			}
			TextBoxConceptUri.Text = GetSuggestedConceptUri(candidate);
		}

		private string GetSuggestedConceptUri(string label)
		{
			if (!_ao.StoreIsValid)
			{
				MessageBox.Show("The annotation ontology is not loaded yet. Please wait.");
				return null;
			}
			if (label != null)
				label = label.Trim();
			if (string.IsNullOrEmpty(label))
				return "";
			string uri = _prefix + Regex.Replace(label, @"\W", "_");
			int existingCount = _ao.Select(new Statement(uri, null, null)).Count();
			if (existingCount == 0)
				return uri;
			// update the uri to make it unique
			int index = 2;
			while (true)
			{
				existingCount = _ao.Select(new Statement(uri + "_" + index, null, null)).Count();
				if (existingCount == 0)
					return uri + "_" + index;
				index++;
			}
		}

		#region loading/saving

		private AnnotationOntology _ao = new AnnotationOntology();
		
		private void SendRequestForTheAnnotationOntology()
		{
			string requestId;
			string queryInfo = String.Format("<query includeComment=\"1\" includeLinksTo=\"1\" />");
			string request = Defaults.BuildRequest(PublisherName, "GetAnnotationOntologyRDF", queryInfo, out requestId);
			SendRequest(request, requestId, "GetAnnotationOntologyRDF");
			Invoke((Action)(() => { StatusText.Text = "Retrieving the annotation ontology concepts from KEUI"; }));

		}
		
		private void LoadOntology(string rdfData)
		{
			_ao.LoadAnnotationOntologyFromString(rdfData);
			_ao.UpdateSuggestionsDict();
			StatusText.Text = "Annotation ontology loaded successfully...";
			Trace.WriteLine(String.Format("Added {0} concepts to the dictionary.", _ao.GetConceptCount()));
			Invoke((Action)(() => { ButtonEditExisting.Enabled = true; }));
		}
		#endregion

		#region helper functs
		public List<TagInfoBase> GetTags(int tagParentId)
		{
			//List<TagInfoBase> tags = new List<TagInfoBase>(from tagInfo in GetTagInfos() where tagInfo.ParentTagId == tagParentId orderby tagInfo.TagName ascending select tagInfo);
			if (_tagIdToChildrenInfo.ContainsKey(tagParentId))
				return _tagIdToChildrenInfo[tagParentId];
			return new List<TagInfoBase>();
		}

		public int GetTagId(int tagParentId, string tagName)
		{
			TagInfoBase tag = GetTag(tagParentId, tagName);
			if (tag != null)
				return tag.TagId;
			return TagInfoBase.InvalidTagId;
		}

		public TagInfoBase GetTag(int tagParentId, string tagName)
		{
			foreach (var tag in GetTags(tagParentId))
			{
				if (tag.TagName == tagName)
					return tag;
			}
			return null;
		}

		public List<Tuple<string, double>> ParseKeywords(string xmlData)
		{
			HtmlAgilityPack.XmlDocument xmlDoc = new HtmlAgilityPack.XmlDocument();
			xmlDoc.LoadXml(xmlData);

			List<Tuple<string, double>> keywords = new List<Tuple<string, double>>();
			HtmlAgilityPack.HtmlNodeCollection kwNodes = xmlDoc.DocumentNode.SelectNodes("/results/keywords//kw");
			if (kwNodes == null) return keywords;

			foreach (var kwNode in kwNodes)
			{
				string text = GenLib.Text.Text.DecodeXMLString(kwNode.GetAttributeValue("str", "").ToLower());
				text = text.Replace("-", "").Replace("_", "");
				if (string.IsNullOrEmpty(text))
					continue;
				if (text.Contains(':'))
					text = text.Substring(text.IndexOf(':') + 1);
				string wgtStr = kwNode.GetAttributeValue("wgt", "0.0");
				double wgt = double.Parse(wgtStr, System.Globalization.NumberFormatInfo.InvariantInfo);
				bool existed = false;
				for (int i = 0; i < keywords.Count; i++)
				{
					var kw = keywords[i];
					if (kw.Item1 == text)
					{
						if (kw.Item2 < wgt)
							keywords[i] = new Tuple<string, double>(text, wgt);
						existed = true;
					}
				}
				if (!existed)
					keywords.Add(new Tuple<string, double>(text, wgt));
			}
			return keywords;
		}

		public List<Tuple<string, double>> ParseNGrams(string xmlData, bool sortByFrequency)
		{
			HtmlAgilityPack.XmlDocument xmlDoc = new HtmlAgilityPack.XmlDocument();
			xmlDoc.LoadXml(xmlData);

			List<Tuple<string, double>> ngrams = new List<Tuple<string, double>>();
			HtmlAgilityPack.HtmlNodeCollection ngramNodes = xmlDoc.DocumentNode.SelectNodes("/results/ngrams//ngram");
			if (ngramNodes == null) return ngrams;

			foreach (var ngramNode in ngramNodes)
			{
				string text = GenLib.Text.Text.DecodeXMLString(ngramNode.GetAttributeValue("text", "").ToLower());
				if (string.IsNullOrEmpty(text))
					continue;
				string fqStr = ngramNode.GetAttributeValue("fq", "0");
				int fq = int.Parse(fqStr);
				ngrams.Add(new Tuple<string, double>(text, fq));
			}
			if (sortByFrequency)
				ngrams.Sort((x, y) =>
				{
					if (x.Item2 > y.Item2) return -1;
					if (x.Item2 < y.Item2) return 1;
					return 0;
				});
			return ngrams;
		}

		private void UpdateGuiState()
		{
			bool labelsValid = !string.IsNullOrEmpty(TextBoxLabels.Text);
			ButtonLearnDescription.Enabled = labelsValid;
			//ButtonAddSelectedConcept.Enabled = valid;
			ButtonManuallyAddRelatedConcepts.Enabled = labelsValid;
			ButtonAutoAddRelatedConcepts.Enabled = !string.IsNullOrEmpty(TextBoxDescription.Text.Trim());

			//ButtonAddSelectedConcept.Enabled = labelsValid && !string.IsNullOrEmpty(TextBoxConceptUri.Text);
			//ButtonGroupAdd.Enabled = ListViewCandidates.SelectedItems.Count > 0;
		}
		#endregion
	}


	// Implements the manual sorting of items by columns.
	class ListViewItemComparer : System.Collections.IComparer
	{
		private int col;
		public ListViewItemComparer()
		{
			col = 0;
		}
		public ListViewItemComparer(int column)
		{
			col = column;
		}
		public int Compare(object x, object y)
		{
			if (((ListViewItem)x).SubItems.Count <= col) return -1;
			if (((ListViewItem)y).SubItems.Count <= col) return 1;
			return String.Compare(((ListViewItem)x).SubItems[col].Text, ((ListViewItem)y).SubItems[col].Text);
		}
	}

	// Implements the manual sorting of items by columns.
	class CandidateItemComparer : System.Collections.IComparer
	{
		public CandidateItemComparer()
		{
		}
		public CandidateItemComparer(int column)
		{
		}
		public int Compare(object x, object y)
		{
			double val1 = (((ListViewItem)x).Tag as Tuple<string, double>).Item2;
			double val2 = (((ListViewItem)y).Tag as Tuple<string, double>).Item2;
			if (val1 < val2) return 1;
			if (val1 > val2) return -1;
			return 0;
		}
	}
}
