using System;
using System.Diagnostics;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml.Linq;
using System.Threading;
using System.Xml.Serialization;
using System.IO;
using System.Text.RegularExpressions;
using System.IO.Compression;
using System.Globalization;

using System.Xml;
using System.Data;
using System.Runtime.InteropServices;
using Contextify.Shared.Types;
using Contextify.Shared.Base;
using HtmlAgilityPack;
using GenLib.Text;
using Contextify.Util;
using Contextify.Shared.Interfaces;

namespace ContextifyServer.Base
{
	public partial class MailData : MailDataSettings, IDisposable
	{
		enum EMinerFAccess { faUndef, faCreate, faUpdate, faAppend, faRdOnly, faRestore }
		Dictionary<int, TagInfoBase> _tagIdToTagInfo = new Dictionary<int, TagInfoBase>();
		Dictionary<string, TagInfoBase> _tagIdStrToTagInfo = new Dictionary<string, TagInfoBase>();
		Dictionary<int, List<TagInfoBase>> _tagIdToChildrenInfo = new Dictionary<int, List<TagInfoBase>>();

		public enum IndexingResult { Success = 0, MailIsNull, BodyIsNull, HeaderOnlyDownloaded, Exception, Ignore };
		public GenLib.Log.LogService.LogInfoDelegate LogInfoHandler = null;
		public void LogInfo(string text) { if (LogInfoHandler != null) LogInfoHandler(text); }

		protected object _serviceLock = new object();
		public int ProfileHandle = -1;

		public PeopleData PeopleData { get; private set; }

		public SettingsServer Settings { get; private set; }
		public ServerProfileSettings ProfileSettings { get; private set; }
		public LoadDataStatusEnum LoadDataStatus { get; private set; }	

		public MailData(Base.SettingsServer settings)
			: base(settings.ProfileFolder)
		{
			try {
				Settings = settings;
				ProfileSettings = new ServerProfileSettings(Settings.ProfileFolder);		
				PeopleData = new PeopleData(this);
			}
			catch (System.Exception ex) {
				GenLib.Log.LogService.LogException("Error in MailData constructor: ", ex);
			}
		}

		private bool _disposed = false;
		public override void Dispose()
		{
			if (_disposed) return;
			try {
				GenLib.Log.LogService.LogInfo("Disposing mail data");

				ProfileSettings.SaveSettings();

				MinerProfileClose();

				PeopleData.Dispose();

				base.Dispose();
			}
			catch (Exception ex) {
				GenLib.Log.LogService.LogException("Disposing mail data exception: ", ex);
			}		
			_disposed = true;
		}

		public virtual LoadDataStatusEnum LoadData()
		{
			LoadDataStatus = LoadDataStatusEnum.Unknown;
			if (!MinerDllValid()) {
				LogInfo("Unable to load data because the ItemMinerLib.dll file is missing.");
				GenLib.Log.LogService.LogWarning("Unable to load data because the ItemMinerLib.dll file is missing.");
				LoadDataStatus = LoadDataStatusEnum.MissingDLLs;
				return LoadDataStatus;
			}

			if (!SQLiteDllValid()) {
				LogInfo("Unable to load data because the System.Data.SQLite.dll file is missing.");
				GenLib.Log.LogService.LogWarning("Unable to load data because the System.Data.SQLite.dll file is missing.");
				LoadDataStatus = LoadDataStatusEnum.MissingDLLs;
				return LoadDataStatus;
			}

			bool peopleDataLoadedOk = PeopleData.LoadData();
			bool mailDataLoadedOk = SQLRecreateTablesIfNotCompatible();

			_tagIdToTagInfo = new Dictionary<int, TagInfoBase>();
			_tagIdStrToTagInfo = new Dictionary<string, TagInfoBase>();
			_tagIdToChildrenInfo = new Dictionary<int, List<TagInfoBase>>();

			int offset = 0;
			int count = 50000;
			while (true) {
				var tagInfos = SQLGetTagInfos(offset, count);
				if (tagInfos.Count() == 0)
					break;
				offset += count;
				foreach (TagInfoBase tagInfo in tagInfos) {
					_tagIdToTagInfo[tagInfo.TagId] = tagInfo;
					if (!string.IsNullOrEmpty(tagInfo.TagIdStr))
						_tagIdStrToTagInfo[tagInfo.TagIdStr] = tagInfo;
					if (!_tagIdToChildrenInfo.ContainsKey(tagInfo.ParentTagId))
						_tagIdToChildrenInfo[tagInfo.ParentTagId] = new List<TagInfoBase>();
					_tagIdToChildrenInfo[tagInfo.ParentTagId].Add(tagInfo);
				}
			}
						
			//Trace.WriteLine("Memory used by tag info: " + (after - before) / 1024 + " KB");

			bool minerDataLoadedOk = false;
			if (mailDataLoadedOk && peopleDataLoadedOk)
				minerDataLoadedOk = MinerProfileLoad();

			if (!Directory.Exists(Settings.MinerDataFolder)) {
				LogInfo("Miner profile data is not existing. Creating a new profile...");
				ClearIndex();
			}	
			else if (!mailDataLoadedOk || !peopleDataLoadedOk) {
				LogInfo("LoadData: Profile data files were not loaded. The current version of SQL tables is not compatible with the current version of contextify server so we are creating a new index...");
				ClearIndex();
			}
			else if (!minerDataLoadedOk) {
				LogInfo("LoadData: Miner data was not loaded. It is possible that the data is missing or that it is not compatible with the current version of the software. We are creating a new index...");
				ClearIndex();
			}
			else
				LogInfo("LoadData: Data Loaded. ProfileHandle = " + ProfileHandle.ToString());

			LoadDataStatus = LoadDataStatusEnum.DataLoadedOK;

			return LoadDataStatus;
		}

		public void ClearIndex()
		{
			try {
				SQLLastClearIndexTime = DateTime.Now;
				base.SQLClearItems();
				PeopleData.SQLClearCounts();

				SQLFullIndexingCompleted = false;

				MinerProfileClose();		
				MinerDeleteMinerFiles();			
				MinerProfileCreateNew();		
				
			}
			catch (Exception ex) { GenLib.Log.LogService.LogException("MailData. ClearIndex: ", ex); }
		}

		public static string UnicodeDefFileLocation = null;

		public string GetUnicodeDefFileName()
		{
			if (!string.IsNullOrEmpty(UnicodeDefFileLocation) && File.Exists(UnicodeDefFileLocation))
				return UnicodeDefFileLocation;
			string path = Settings.GetModulePath();
			while (true) {
				try {
					string unicodeDefFile = Path.Combine(path, "UnicodeDef.Bin");
					if (File.Exists(unicodeDefFile))
						return unicodeDefFile;
					path = Directory.GetParent(path).FullName;
				}
				catch {
					return null;
				}
			}
		}

		#region item related functions
		public string GetItemBody(int itemId)
		{
			return SQLGetItemBody(itemId);
		}

		public string GetItemMetaData(int itemId)
		{
			return SQLGetItemMetaData(itemId);
		}

		public void GetItemBodyAsync(int itemId, QueryCallback queryCallback)
		{
			string body = SQLGetItemBody(itemId);
			queryCallback(body);
		}

		public void GetItemMetaDataAsync(int itemId, QueryCallback queryCallback)
		{
			string body = SQLGetItemMetaData(itemId);
			queryCallback(body);
		}

		public string GetItemInfo(int itemId = -1, string entryId = null)
		{
			DataRow data = null;
			if (itemId != -1)
				data = SQLGetItem(itemId);
			else if (!string.IsNullOrEmpty(entryId))
				data = SQLGetItem(entryId);
			if (data == null)
				return null;
			if (itemId == -1)
				itemId = (int)(long)data[0];

			GeneralQuery generalQuery = new GeneralQuery(new QGeneralParams() { MaxCount = 1, IncludeAttachments = false, ItemDataSnipLen = -1 }, new QArgs(new QItemIdCond(itemId)));
			string ret = MinerQuery(generalQuery);
			if (string.IsNullOrEmpty(ret))
				return null;
			HtmlAgilityPack.XmlDocument doc = new HtmlAgilityPack.XmlDocument();
			doc.LoadXml(ret);
			HtmlAgilityPack.HtmlNode itemNode = doc.DocumentNode.SelectSingleNode("//item");
			if (itemNode == null)
				return null;
			itemNode.SetAttributeValue("accountId", ((long)data[2]).ToString());
			itemNode.SetAttributeValue("itemState", ((byte)data[4]).ToString());

			return itemNode.OuterHtml;
		}
		#endregion

				
		#region tag-related functions
		public static readonly int TagIdRoot = -1;
		public static readonly int TagIdEmails = 0;
		public static readonly int TagIdAttachments = 1;
		public static readonly int TagIdDocuments = 2;
		public static readonly int TagIdSocialNetworks = 9;
		public static readonly int TagIdFacebook = 10;
		public static readonly int TagIdLinkedIn = 11;
		public static readonly int TagIdTwitter = 12;
		public static readonly int TagIdCustomTags = 29;
		public static readonly int TagIdStarred = 30;
		public static readonly int TagIdRelevant = 31;
		public static readonly int TagIdLater = 32;
		public static readonly int TagIdHide = 33;

		enum QueryTypeEnum
		{
			GeneralQuery = 0,
			CustomQuery
		};

		public int CreateTag(string tagName, string tagIdStr, int parentTagId, string tagMeta = null)
		{
			int tagId = SQLCreateTag(tagName, tagIdStr, parentTagId, tagMeta);
			TagInfoBase tag = new TagInfoBase(tagName, tagId, tagIdStr, parentTagId, tagMeta);
			_tagIdToTagInfo[tagId] = tag;
			if (!string.IsNullOrEmpty(tag.TagIdStr))
				_tagIdStrToTagInfo[tag.TagIdStr] = tag;
			if (!_tagIdToChildrenInfo.ContainsKey(parentTagId))
				_tagIdToChildrenInfo[parentTagId] = new List<TagInfoBase>();
			_tagIdToChildrenInfo[parentTagId].Add(tag);
			return tagId;
		}

		public void RenameTag(int tagId, string newName)
		{
			TagInfoBase tag = GetTagInfo(tagId);
			if (tag != null)
				tag.TagName = newName;
			SQLRenameTag(tagId, newName);
		}

		public void DeleteTag(int tagId)
		{
			TagInfoBase tag = _tagIdToTagInfo[tagId];
			if (tag != null && _tagIdStrToTagInfo.ContainsKey(tag.TagIdStr))
				_tagIdStrToTagInfo.Remove(tag.TagIdStr);
			if (tag != null && _tagIdToChildrenInfo.ContainsKey(tag.ParentTagId) && _tagIdToChildrenInfo[tag.ParentTagId].Contains(tag))
				_tagIdToChildrenInfo[tag.ParentTagId].Remove(tag);
			if (_tagIdToTagInfo.ContainsKey(tagId))
				_tagIdToTagInfo.Remove(tagId);
		}

		public bool IsTagId(int tagId)
		{
			return _tagIdToTagInfo.ContainsKey(tagId);
		}

		public bool IsTagIdStr(int tagId)
		{
			return _tagIdToTagInfo.ContainsKey(tagId);
		}

		public int CreateTagIfNotExisting(string tagName, string tagIdStr, int parentTagId, string tagMeta = null)
		{
			Debug.Assert(!string.IsNullOrEmpty(tagName), "tag name is empty or null");
			Debug.Assert(!string.IsNullOrEmpty(tagIdStr), "tagIdStr name is empty or null");
			TagInfoBase existingTagInfo = GetTagInfo(tagIdStr);
			if (existingTagInfo != null) {
				if (existingTagInfo.TagName != tagName)
					RenameTag(existingTagInfo.TagId, tagName);
				return existingTagInfo.TagId;
			}
			int tag = CreateTag(tagName, tagIdStr, parentTagId, tagMeta);
			return tag;
		}

		public int GetTagId(int tagParentId, string tagName)
		{
			TagInfoBase tag = GetTagInfo(tagParentId, tagName);
			if (tag != null)
				return tag.TagId;
			else
				return TagInfoBase.InvalidTagId;
		}

		public int GetTagId(string tagIdStr)
		{
			TagInfoBase tag = GetTagInfo(tagIdStr);
			if (tag != null)
				return tag.TagId;
			else
				return TagInfoBase.InvalidTagId;
		}

		public IEnumerable<TagInfoBase> GetTags(int tagParentId)
		{
			if (_tagIdToChildrenInfo.ContainsKey(tagParentId))
				return _tagIdToChildrenInfo[tagParentId];
			return new TagInfoBase[0];
		}

		public string GetTagName(int tagId)
		{
			Debug.Assert(IsTagId(tagId));
			return _tagIdToTagInfo[tagId].TagName;
		}

		public string GetTagName(string tagIdStr)
		{
			if (_tagIdStrToTagInfo.ContainsKey(tagIdStr))
				return _tagIdStrToTagInfo[tagIdStr].TagName;
			return null;
		}

		public int GetParentTagId(int tagId)
		{
			Debug.Assert(IsTagId(tagId));
			return _tagIdToTagInfo[tagId].ParentTagId;
		}

		public string GetTagIdStr(int tagId)
		{
			Debug.Assert(IsTagId(tagId));
			return _tagIdToTagInfo[tagId].TagIdStr;
		}

		public TagInfoBase GetTagInfo(int tagId)
		{
			try {
				if (_tagIdToTagInfo.ContainsKey(tagId))
					return _tagIdToTagInfo[tagId];
			}
			catch (Exception ex) {
				GenLib.Log.LogService.LogException("MailData. GetTagInfo exception: ", ex);
			}
			return null;
		}

		public TagInfoBase GetTagInfo(string tagIdStr)
		{
			try {
				if (_tagIdStrToTagInfo.ContainsKey(tagIdStr))
					return _tagIdStrToTagInfo[tagIdStr];
			}
			catch (Exception ex) {
				GenLib.Log.LogService.LogException("MailData. GetTagInfo exception: ", ex);
			}
			return null;
		}

		public TagInfoBase GetTagInfo(int tagParentId, string tagName)
		{
			if (_tagIdToChildrenInfo.ContainsKey(tagParentId)) {
				foreach (var tagInfo in _tagIdToChildrenInfo[tagParentId]) {
					if (tagInfo.TagName == tagName)
						return tagInfo;
				}
			}
			return null;
		}

		public string GetTagMeta(int tagId)
		{
			Debug.Assert(IsTagId(tagId));
			return _tagIdToTagInfo[tagId].TagMeta;
		}

		public string GetTagMeta(string tagIdStr)
		{
			if (_tagIdStrToTagInfo.ContainsKey(tagIdStr))
				return _tagIdStrToTagInfo[tagIdStr].TagMeta;
			return null;
		}

		public IEnumerable<int> GetItemsForTagId(int tagId)
		{
			try {
				string data = MinerGetItemsForTagId(tagId);
				if (data == null)
					return new int[0];
				return from string idStr in data.Split(",".ToCharArray(), StringSplitOptions.RemoveEmptyEntries) select int.Parse(idStr);
			}
			catch (Exception ex) {
				GenLib.Log.LogService.LogException("MailData. GetItemIdsForTag: ", ex);
			}
			return new int[0];
		}

		public IEnumerable<int> GetTagsForItemId(int itemId)
		{
			try {
				string data = MinerGetTagsForItemId(itemId);
				if (data == null)
					return new int[0];
				return from string tagIdStr in data.Split(",".ToCharArray(), StringSplitOptions.RemoveEmptyEntries) select int.Parse(tagIdStr);
			}
			catch (Exception ex) {
				GenLib.Log.LogService.LogException("MailData. GetTagsForItemId: ", ex);
			}
			return new int[0];
		}

		public IEnumerable<int> GetTagsWithParentTag(IEnumerable<int> tagIds, int parentTagId)
		{
			return from tagId in tagIds where GetParentTagId(tagId) == parentTagId select tagId;
		}
		
		public int GetTagIdStarred() { return TagIdStarred; }
		#endregion

		#region Functions that use miner
		HashSet<string> _recipientRoles = new HashSet<string>() { "to", "cc", "bcc" };
		HashSet<string> _senderRoles = new HashSet<string>() { "from", "author" };
		public AddItemStatus AddItem(string itemInfo, string itemContent)
		{
			try {
				itemInfo = Text.RemoveControlCharacters(itemInfo);
				itemContent = Text.RemoveControlCharacters(itemContent);
				if (string.IsNullOrEmpty(itemInfo))
					return AddItemStatus.GetInvalidItemStatus("ItemInfo parameter is empty. Unable to add item to the index");

				HtmlAgilityPack.XmlDocument itemInfoDoc = new HtmlAgilityPack.XmlDocument();
				itemInfoDoc.LoadXml(itemInfo);
				bool editedItemInfo = false;

				HtmlNode itemNode = itemInfoDoc.DocumentNode.SelectSingleNode("/item");
				if (itemNode.GetAttributeValue("itemId", -1) == -1) {
					ESQLItemType itemType = (ESQLItemType)itemNode.GetAttributeValue("itemType", 0);
					string entryId = itemInfoDoc.DocumentNode.SelectSingleNode("/item/entryId").InnerText.DecodeXMLString();
					int accountId = itemNode.GetAttributeValue("accountId", -1);
					SQLAddItem(entryId, itemType, accountId, ESQLItemState.ToBeAdded);
					int computedItemId = SQLGetItemId(entryId);
					itemNode.SetAttributeValue("itemId", computedItemId.ToString());
					editedItemInfo = true;
				}

				if (itemInfoDoc.DocumentNode.SelectNodes("/item/people/person[@nameTrust]") != null) {
					foreach (var node in itemInfoDoc.DocumentNode.SelectNodes("/item/people/person[@nameTrust]")) {
						string account = node.GetAttributeValue("account", null);
						string name = node.GetAttributeValue("name", null);
						name = name.DecodeXMLString();
						account = account.DecodeXMLString();
						int accountType = node.GetAttributeValue("accountType", -1);
						int nameTrust = node.GetAttributeValue("nameTrust", -1);
						if (name == null || account == null || accountType == -1 || nameTrust == -1) {
							GenLib.Log.LogService.LogError("Item was not added to the index due to missing information in the people section. Invalid data is: " + node.InnerHtml);
							return AddItemStatus.GetInvalidItemStatus("Item was not added to the index due to missing information in the people section. Invalid data is: " + node.InnerHtml);
						}
						PeopleData.AddNewPersonOrUpdate(name, account, (EAccountType)accountType, (EPersonNameTrust)nameTrust);
						int accountId = PeopleData.GetAccountId(account, (EAccountType)accountType);
						node.SetAttributeValue("id", accountId.ToString());
						editedItemInfo = true;
					}
				}

				if (editedItemInfo == true)
					itemInfo = itemInfoDoc.DocumentNode.InnerHtml;

				int senderId = -1;
				List<int> recipientIds = new List<int>();
				foreach (var node in itemInfoDoc.DocumentNode.SelectNodes("/item/people/person") ?? new HtmlNodeCollection(null)) {
					if (_senderRoles.Contains(node.GetAttributeValue("role", null))) senderId = node.GetAttributeValue("id", -1);
					else if (_recipientRoles.Contains(node.GetAttributeValue("role", null))) recipientIds.Add(node.GetAttributeValue("id", -1));
				}
				if (senderId != -1) {
					PeopleData.IncreaseCountInfo(senderId, recipientIds);
				}

				var subjectNode = itemNode.SelectSingleNode("./subject");
				string subject = subjectNode != null ? subjectNode.InnerText.DecodeXMLString() : "";
				var metaNode = itemNode.SelectSingleNode("./metaData");
				string meta = metaNode != null ? metaNode.InnerHtml : "";
				
				if (Settings.StoreToFileAddItemInfo)
					StoreToFile("itemInfo", itemInfo);
				if (Settings.StoreToFileAddItemContent)
					StoreToFile("itemContent", itemContent);
				AddItemStatus itemStatus = MinerAddItem(itemInfo, itemContent);
				if (itemStatus.ItemId != -1)
				{
					SQLSetItemState(itemStatus.ItemId, ESQLItemState.Indexed);
					SQLSetItemSubjectBodyMeta(itemStatus.ItemId, subject, itemContent, meta);
				}
				else
					LogInfo("Indexing of the item failed. Error info: " + itemStatus.ErrorMessage);
				return itemStatus;
			}
			catch (Exception ex) { 
				GenLib.Log.LogService.LogException("MailData. AddItem: ", ex);
				return AddItemStatus.GetInvalidItemStatus("MailData AddItem exception: " + ex.Message, ex.StackTrace);
			}
		}

		private bool MinerDllValid()
		{
			try {
				lock (_serviceLock) {
					return ItemMinerControl.ItemMiner.DllValid();
				}
			}
			catch (System.DllNotFoundException e) {
				GenLib.Log.LogService.LogException("MailData.MinerDllValid exception: ", e);
			}
			catch (Exception ex) {
				GenLib.Log.LogService.LogException("MinerData.MinerDllValid exception: ", ex);
			}
			return false;
		}

		private bool MinerProfileCreateNew()
		{
			try {
				if (!Directory.Exists(Settings.MinerDataFolder))
					Directory.CreateDirectory(Settings.MinerDataFolder);
			}
			catch (Exception) { GenLib.Log.LogService.LogError("Unable to create directory " + Settings.MinerDataFolder); }

			Debug.Assert(MinerDllValid());
			string unicodeDefFile = GetUnicodeDefFileName();
			if (!File.Exists(unicodeDefFile)) {
				LogInfo("Unable to find the UnicodeDef.Bin file. Please make sure it is in the folder with the application or some folder upstream.");
				return false;
			}

			DateTime start = DateTime.Now;
			try {
				lock (_serviceLock) {
					ProfileHandle = ItemMinerControl.ItemMiner.ProfileNew(Settings.MinerDataFolder + Path.DirectorySeparatorChar, unicodeDefFile, ProfileSettings.MinerMxNGramLen, ProfileSettings.MinerMxCachedNGrams, ProfileSettings.MinerIndexCacheSizeMB, ProfileSettings.MinerItemCacheSizeMB);
					//LogEvent("ContextifyTrayWindow::CreateNewProfileFiles: New profile files created.", false);
				}
				MinerUpdateSettings();
			}
			catch (Exception ex) {
				LogInfo("MinerData.MinerProfileCreateNew exception: " + ex.Message);
				GenLib.Log.LogService.LogException("MinerData.MinerProfileCreateNew exception: ", ex);
			}
			return ProfileHandle != -1;
		}

		private bool MinerProfileLoad()
		{
			DateTime start = DateTime.Now;
			try {
				Debug.Assert(MinerDllValid());
				if (!Directory.Exists(Settings.MinerDataFolder))
					return false;

				VersionDataServer currentVersion = new VersionDataServer();
				string loadedMailVersion = "";
				VersionDataServer loadedVersion = SettingsServer.GetVersionData(Settings.ProfileFolder);
				if (loadedVersion != null)
					loadedMailVersion = loadedVersion.MailDataVersion;

				//System.Diagnostics.Debugger.Break();
				int storeFileCount = Directory.EnumerateFiles(Settings.MinerDataFolder, "*.Store").Count();
				bool indexGixExists = File.Exists(Path.Combine(Settings.MinerDataFolder, "Index.Gix"));
				string unicodeDefFile = GetUnicodeDefFileName();
				if (!File.Exists(unicodeDefFile)) {
					LogInfo("Unable to find the UnicodeDef.Bin file. Please make sure it is in the folder with the KEUI application or some folder upstream.");
					return false;
				}

				if (storeFileCount == 0) {
					LogInfo("There were no miner stores found. Unable to load miner data.");
					return false;
				}

				if (currentVersion.MailDataVersion != loadedMailVersion) {
					LogInfo("Miner contains information from previous version and is unable to load it.");
					return false;
				}

				if (indexGixExists == false) {
					LogInfo("The miner index files don't exist. Loading of miner data failed.");
					return false;
				}

				lock (_serviceLock) {
					ProfileHandle = ItemMinerControl.ItemMiner.ProfileLoad(Settings.MinerDataFolder + Path.DirectorySeparatorChar, unicodeDefFile, (int)EMinerFAccess.faUpdate, ProfileSettings.MinerIndexCacheSizeMB, ProfileSettings.MinerItemCacheSizeMB);
				}
				GenLib.Log.LogService.LogInfo(String.Format("Loading miner data needed {0:f0} miliseconds", (DateTime.Now - start).TotalMilliseconds));
				LogInfo("Loaded profile " + Settings.ProfileFolderName + ". Profile is in folder " + Settings.ProfileFolder);
			}
			catch (Exception ex) {
				LogInfo("MinerData.MinerProfileLoad exception: " + ex.Message);
				GenLib.Log.LogService.LogException("MinerData.MinerProfileLoad exception: ", ex);
			}
			return ProfileHandle != -1;
		}

		public void MinerProfileClose()
		{
			try {
				//LogEvent("Closing profile " + Settings.CurrentProfile);
				lock (_serviceLock) {
					if (ProfileHandle != -1)
						ItemMinerControl.ItemMiner.ProfileClose(ProfileHandle);
					ProfileHandle = -1;
				}

			}
			catch (Exception ex) {
				GenLib.Log.LogService.LogException("MailData. MinerProfileClose: ", ex);
				LogInfo("MailData. MinerProfileClose: " + ex.Message);
			}
		}


		private AddItemStatus MinerAddItem(String itemInfo, String itemContent)
		{
			if (ProfileHandle == -1) {
				return AddItemStatus.GetInvalidItemStatus("Unable to add item. Profile handle is invalid");
			}
			try {
				DateTime start = DateTime.Now;
				string status;
				lock (_serviceLock) {
					status = ItemMinerControl.ItemMiner.AddItem(ProfileHandle, itemInfo, itemContent);
					//File.WriteAllText(@"E:\d\" + index.ToString() + "-info.txt", itemInfo);
					//File.WriteAllText(@"E:\d\" + index.ToString() + "-content.txt", itemContent);
					//index++;
				}
				AddItemStatus itemStatus = new AddItemStatus(status);
				//StrFormatByteSize(itemContent.Length, _itemSize, 100);
				GenLib.Log.LogService.LogInfo(String.Format("MinerAddItem: docId = {0:N0}, content size = {1:N0}, time needed = {2:N0} ms", itemStatus.ItemId, itemContent.Length.ToString(), (int)(DateTime.Now - start).TotalMilliseconds));
				return itemStatus;
			}
			catch (Exception ex) { 
				GenLib.Log.LogService.LogException("MailData. MinerAddItem: ", ex);
				return AddItemStatus.GetInvalidItemStatus("MinerAddItem exception: " + ex.Message, ex.StackTrace);
			}
		}

		public void MinerSetTag(int tagId, int itemId)
		{
			if (ProfileHandle == -1) return;
			try {
				lock (_serviceLock) {
					ItemMinerControl.ItemMiner.SetTag(ProfileHandle, itemId, tagId.ToString());
				}
			}
			catch (Exception ex) { GenLib.Log.LogService.LogException("MailData. MinerSetTag: ", ex); }
		}

		public void MinerRemoveTag(int tagId, int itemId)
		{
			if (ProfileHandle == -1) return;
			try {
				lock (_serviceLock) {
					ItemMinerControl.ItemMiner.RemoveTag(ProfileHandle, itemId, tagId.ToString());
				}
			}
			catch (Exception ex) { GenLib.Log.LogService.LogException("MailData. MinerRemoveTag: ", ex); }
		}

		public void MinerRemoveItems(IEnumerable<int> itemIdList)
		{
			if (ProfileHandle == -1) return;
			try {
				string idListStr = string.Join(",", itemIdList);
				lock (_serviceLock) {

					ItemMinerControl.ItemMiner.RemoveItems(ProfileHandle, String.Format("<removeItems>{0}</removeItems>", idListStr));
				}
			}
			catch (Exception ex) { GenLib.Log.LogService.LogException("MailData. MinerDeleteItem: ", ex); }
		}

		public void MinerRemoveItem(int itemId)
		{
			if (ProfileHandle == -1) return;
			try {
				lock (_serviceLock) {
					ItemMinerControl.ItemMiner.RemoveItem(ProfileHandle, itemId);
				}
			}
			catch (Exception ex) { GenLib.Log.LogService.LogException("MailData. MinerDeleteItem: ", ex); }
		}

		public bool MinerIsProfileValid()
		{
			return (ProfileHandle != -1);
		}

		public string MinerQuery(QueryBase query)
		{
			if (ProfileHandle == -1) return null;
			Stopwatch stopwatch = new Stopwatch();
			stopwatch.Start();
			if (Settings.StoreToFileQueryRequest)
				StoreToFile("request", query.ToString());
			string ret = null;
			try {
				DateTime start = DateTime.Now;
				lock (_serviceLock) {
					ret = ItemMinerControl.ItemMiner.Query(ProfileHandle, query.ToString());
				}
			}
			catch (Exception ex) {
				GenLib.Log.LogService.LogException("MailData. MinerQuery: ", ex);
				LogInfo("MailData. MinerQuery: " + ex.Message);
			}
			
			ret = AddItemContent(query, ret);
			
			bool includePeopleData = false;
			if (query is GeneralQuery) includePeopleData = (query as GeneralQuery).QueryParams.IncludePeopleData;
			else if (query is CustomQuery) includePeopleData = (query as CustomQuery).QueryParams.IncludePeopleData;
			if (includePeopleData)
				ret = AddPeopleInfoToResults(ret);
			if (Settings.StoreToFileQueryResponse)
				StoreToFile("response", ret);
			string queryType = "";
			if (query is GeneralQuery) queryType = (query as GeneralQuery).QueryParams.ResultData.ToString();
			else if (query is CustomQuery) queryType = (query as CustomQuery).QueryParams.GetType().Name;
			LogInfo(String.Format("MailData. MinerQuery for query type {0} took {1} ms", queryType, stopwatch.ElapsedMilliseconds));
			return ret;
		}


		public string MinerGetLastInformation()
		{
			try {
				lock (_serviceLock) {
					return ItemMinerControl.ItemMiner.GetLastInformation(ProfileHandle);
				}
			}
			catch (Exception ex) { GenLib.Log.LogService.LogException("MailData. MinerGetLastInformation: ", ex); }
			return "";
		}

		public string MinerGetStatus()
		{
			try {
				lock (_serviceLock) {
					return ItemMinerControl.ItemMiner.GetStatus(ProfileHandle);
				}
			}
			catch (Exception ex) { GenLib.Log.LogService.LogException("MailData. MinerGetStatus: ", ex); }
			return "";
		}

		public string MinerGetItemsForTagId(int tagId)
		{
			if (ProfileHandle == -1) return null;
			try {
				string request = String.Format("<tagData tagId=\"{0}\" />", tagId);
				lock (_serviceLock) {
					return ItemMinerControl.ItemMiner.GetTagData(ProfileHandle, request);
				}
			}
			catch (Exception ex) { GenLib.Log.LogService.LogException("MailData. MinerGetItemsForTag: ", ex); }
			return null;
		}

		public string MinerGetTagsForItemId(int itemId)
		{
			if (ProfileHandle == -1) return null;
			try {
				string request = String.Format("<tagData itemId=\"{0}\" />", itemId);
				lock (_serviceLock) {
					return ItemMinerControl.ItemMiner.GetTagData(ProfileHandle, request);
				}
			}
			catch (Exception ex) { GenLib.Log.LogService.LogException("MailData. MinerGetTagsForItem: ", ex); }
			return null;
		}

		public string MinerGetTagData(string tagInfo)
		{
			if (ProfileHandle == -1) return null;
			try {
				lock (_serviceLock) {
					return ItemMinerControl.ItemMiner.GetTagData(ProfileHandle, tagInfo);
				}
			}
			catch (Exception ex) { GenLib.Log.LogService.LogException("MailData. MinerGetTagData: ", ex); }
			return null;
		}

		public bool MinerSetTagForItems(int tagId, IEnumerable<int> itemIds)
		{
			if (ProfileHandle == -1) return false;
			string itemsStr = string.Join(",", itemIds);
			string queryStr = String.Format("<tagData operation=\"add\" tagId=\"{0}\">{1}</tagData> ", tagId, itemsStr);
			try {
				lock (_serviceLock) {
					return ItemMinerControl.ItemMiner.SetTagData(ProfileHandle, queryStr);
				}
			}
			catch (Exception ex) { GenLib.Log.LogService.LogException("MailData. MinerSetTagForItems: ", ex); }
			return false;
		}

		public bool MinerRemoveTagForItems(int tagId, IEnumerable<int> itemIds)
		{
			if (ProfileHandle == -1) return false;
			string itemsStr = string.Join(",", itemIds);
			string queryStr = String.Format("<tagData operation=\"remove\" tagId=\"{0}\">{1}</tagData> ", tagId, itemsStr);
			try {
				lock (_serviceLock) {
					return ItemMinerControl.ItemMiner.SetTagData(ProfileHandle, queryStr);
				}
			}
			catch (Exception ex) { GenLib.Log.LogService.LogException("MailData. MinerRemoveTagForItems: ", ex); }
			return false;
		}

		public string MinerGetTopWords(int wordCount = 100, bool groupByThreads = true, int maxNGramLen = 2, int minNGramFq = 10)
		{
			string ret = null;
			try {
				lock (_serviceLock) {
					ret = ItemMinerControl.ItemMiner.GetTopWords(ProfileHandle, wordCount, groupByThreads, maxNGramLen, minNGramFq);
				}
			}
			catch (Exception ex) { GenLib.Log.LogService.LogException("MailData. MinerGetTopWords: ", ex); }
			return ret;
		}

		private void MinerDeleteMinerFiles()
		{
			try {
				if (Directory.Exists(Settings.MinerDataFolder)) {
					foreach (string filename in Directory.GetFiles(Settings.MinerDataFolder))
						GenLib.IO.SafeDeleteFile(filename);
				}
			}
			catch (Exception ex) {
				GenLib.Log.LogService.LogException("MailData. MinerDeleteMinerFiles: ", ex);
				LogInfo("MailData. MinerDeleteMinerFiles: " + ex.Message);
			}
		}

		public string MinerExecuteCommand(string command)
		{
			try {
				lock (_serviceLock) {
					return ItemMinerControl.ItemMiner.ExecuteCommand(ProfileHandle, command);
				}
			}
			catch (Exception ex) { GenLib.Log.LogService.LogException("MailData. MinerExecuteCommand: ", ex); }
			return "";
		}

		public string MinerTokenizeText(string text)
		{
			try {
				lock (_serviceLock) {
					return ItemMinerControl.ItemMiner.TokenizeText(ProfileHandle, text);
				}
			}
			catch (Exception ex) { GenLib.Log.LogService.LogException("MailData. MinerExecuteCommand: ", ex); }
			return "";
		}

		public void MinerUpdateSettings()
		{
			try {
				lock (_serviceLock) {
					string minerSettings = String.Format("<Settings updateThreadBow=\"{0}\" updateNGrams=\"{1}\" nGramsIgnoreSw=\"{2}\" />", ProfileSettings.MinerUpdateThreadBow ? 1 : 0, ProfileSettings.MinerUpdateNGrams ? 1 : 0, ProfileSettings.MinerNGramsIgnoreSw ? 1 : 0);
					ItemMinerControl.ItemMiner.UpdateSettings(ProfileHandle, minerSettings);
				}
			}
			catch (Exception ex) { GenLib.Log.LogService.LogException("MailData. MinerUpdateSettings: ", ex); }
		}

		private string AddItemContent(QueryBase query, string returnData)
		{
			if (string.IsNullOrEmpty(returnData))
				return returnData;
			
			bool addItemData = false;
			int itemDataSnipLen = -1;
			bool snipMatchKeywords = true;
			int keywordMatchOffset = 25;
			List<string> keywords = new List<string>();

			if (query is GeneralQuery) {
				GeneralQuery generalQuery = query as GeneralQuery;
				addItemData = generalQuery.QueryParams.ResultData.HasFlag(QResultData.itemData);
				itemDataSnipLen = generalQuery.QueryParams.ItemDataSnipLen;
				snipMatchKeywords = generalQuery.QueryParams.SnipMatchKeywords;
				keywordMatchOffset = generalQuery.QueryParams.KeywordMatchOffset;
				foreach (QKeywordCond kwCond in from arg in generalQuery.QueryArgs.Conditions where arg is QKeywordCond select arg)
					keywords.AddRange(from kw in kwCond.Keywords select kw.Keyword);
			}
			else if (query is CustomQuery) {
				CustomQuery customQuery = query as CustomQuery;
				if (customQuery.QueryParams is QSimilarThreadsParam) {
					addItemData = (customQuery.QueryParams as QSimilarThreadsParam).IncludeItemData;
					itemDataSnipLen = (customQuery.QueryParams as QSimilarThreadsParam).ItemDataSnipLen;
				}
				else if (customQuery.QueryParams is QSimilarItemsParam) {
					addItemData = (customQuery.QueryParams as QSimilarItemsParam).IncludeItemData;
					itemDataSnipLen = (customQuery.QueryParams as QSimilarItemsParam).ItemDataSnipLen;
				}
				foreach (QKeywordCond kwCond in from arg in customQuery.QueryArgs.Conditions where arg is QKeywordCond select arg)
					keywords.AddRange(from kw in kwCond.Keywords select kw.Keyword);
			}

			if (!addItemData) 
				return returnData;
			
			HtmlAgilityPack.XmlDocument returnDoc = new HtmlAgilityPack.XmlDocument();
			returnDoc.LoadXml(returnData);

			foreach (HtmlAgilityPack.HtmlNode itemNode in returnDoc.DocumentNode.SelectNodes("//item") ?? new HtmlNodeCollection(null)) {
				try {
					int itemId = itemNode.GetAttributeValue("id", -1);
					if (itemId == -1)
						continue;
					Tuple<string, string, string> subjectBodyMeta = SQLGetItemSubjectBodyMeta(itemId);
					string subject = subjectBodyMeta.Item1;
					string body = subjectBodyMeta.Item2;
					string meta = subjectBodyMeta.Item3;

					int firstIndex = -1;
					string title;
					
					if (itemDataSnipLen != -1) {
						for (int i = 0; i < keywords.Count && i < 10; i++) {
							int index = body.IndexOf(keywords[i], StringComparison.InvariantCultureIgnoreCase);
							if (index >= 0 && (firstIndex == -1 || index < firstIndex))
								firstIndex = index;
						}
						if (firstIndex > 0)
							firstIndex -= keywordMatchOffset;
						while (firstIndex > 0 && !char.IsWhiteSpace(body[firstIndex]))
							firstIndex--;
						if (firstIndex + itemDataSnipLen > body.Length)
							firstIndex = body.Length - itemDataSnipLen;
						firstIndex = Math.Max(0, firstIndex);
						body = body.Substring(firstIndex, Math.Min(itemDataSnipLen, body.Length - firstIndex));
						title = "shortContent";
					}
					else
						title = "fullContent";
					HtmlNode bodyNode = returnDoc.CreateElement(title);
					bodyNode.AppendChild(returnDoc.CreateTextNode(body.EncodeXMLString()));
					itemNode.AppendChild(bodyNode);

					HtmlNode subjectNode = returnDoc.CreateElement("subject");
					subjectNode.AppendChild(returnDoc.CreateTextNode(subject.EncodeXMLString()));
					itemNode.AppendChild(subjectNode);

					HtmlNode metaNode = returnDoc.CreateElement("metaData");
					metaNode.AppendChild(returnDoc.CreateTextNode(meta));
					itemNode.AppendChild(metaNode);
				}
				catch (Exception ex) {
					GenLib.Log.LogService.LogException("MailData. AddItemContent: ", ex);
				}
			}

			returnData = returnDoc.DocumentNode.InnerHtml;
			return returnData;
		}

		public string AddPeopleInfoToResults(string returnData)
		{
			try {
				if (string.IsNullOrEmpty(returnData))
					return returnData;

				HtmlAgilityPack.XmlDocument returnDoc = new HtmlAgilityPack.XmlDocument();
				returnDoc.LoadXml(returnData);

				char[] minusArray = "-".ToCharArray();
				char[] commaArray = ",".ToCharArray();
				HashSet<int> detectedAccountIds = new HashSet<int>();

				string[] roles = ProfileSettings.PeopleRoles.Split(commaArray, StringSplitOptions.RemoveEmptyEntries);

				foreach (HtmlAgilityPack.HtmlNode itemNode in returnDoc.DocumentNode.SelectNodes("//item") ?? new HtmlNodeCollection(null)) {
					foreach (string role in roles) {
						string[] ids = itemNode.GetAttributeValue(role, "").Split(commaArray, StringSplitOptions.RemoveEmptyEntries);
						foreach (string id in ids)
							detectedAccountIds.Add(int.Parse(id));
					}
				}

				var peopleNode = returnDoc.DocumentNode.SelectSingleNode("/results/fromToCounts");
				if (peopleNode != null) {
					string[] items = peopleNode.InnerText.RemoveWhiteSpaces().Split(commaArray, StringSplitOptions.RemoveEmptyEntries);
					foreach (string item in items) {
						string[] parts = item.Split(minusArray, StringSplitOptions.RemoveEmptyEntries);
						detectedAccountIds.Add(int.Parse(parts[0]));
						detectedAccountIds.Add(int.Parse(parts[1]));
					}
				}

				if (detectedAccountIds.Count == 0)
					return returnData;

				StringBuilder peopleInfo = new StringBuilder();
				peopleInfo.Append("\t<peopleData>\n");
				foreach (int id in detectedAccountIds) {
					IPersonInfo p = PeopleData.GetPersonFromAccount(id);
					peopleInfo.Append(String.Format("\t\t<person id=\"{0}\" account=\"{1}\" name=\"{2}\" uuid=\"{3}\" />\n", id, PeopleData.GetAccount(id).EncodeXMLString(), p.Name.EncodeXMLString(), p.CustomData.EncodeXMLString()));
				}
				peopleInfo.Append("\t</peopleData>\n");

				Match resultsMatch = Regex.Match(returnData, @"<results[^>]*>\n");
				Trace.Assert(resultsMatch.Success == true, "Unable to find <results> section in the result data");
				int insertIndex = resultsMatch.Index + resultsMatch.Length;
				return returnData.Insert(insertIndex, peopleInfo.ToString());
			}
			catch (Exception ex) {
				GenLib.Log.LogService.LogException("MailData. AddPeopleInfoToResults: ", ex);
			}
			return "";
		}

		#endregion

		#region client-server functions
		public void RemoveItems(IEnumerable<int> itemIds)
		{
			SQLRemoveItems(itemIds);
			MinerRemoveItems(itemIds);
		}

		public void RemoveItems(IEnumerable<string> entryIds)
		{
			IEnumerable<int> itemIds = from entryId in entryIds select SQLGetItemId(entryId);
			RemoveItems(from itemId in itemIds where itemId != -1 select itemId);
		}

		public IEnumerable<TagInfoBase> GetTagInfoList()
		{
			try {
				return from tag in _tagIdToTagInfo.Values select tag;
			}
			catch (Exception ex) {
				GenLib.Log.LogService.LogException("MailData. GetTagInfoList exception: ", ex);
				return new TagInfoBase[0];
			}
		}

		public string RequestTagData(string content, HtmlAgilityPack.HtmlNode dataQueryNode)
		{
			StringBuilder result = new StringBuilder();
			if (content == "GetItemsForTagId") {
				int tagId = dataQueryNode.GetAttributeValue("tagId", -1);
				return MinerGetItemsForTagId(tagId);
			}
			else if (content == "GetTagsForItemId") {
				int itemId = dataQueryNode.GetAttributeValue("itemId", -1);
				return MinerGetTagsForItemId(itemId);
			}
			return null;
		}

		public string RequestData(string content, HtmlAgilityPack.HtmlNode dataQueryNode)
		{
			return null;
		}

		public string SetData(string content, HtmlAgilityPack.HtmlNode data)
		{
			return "";
		}

		public string SetTagData(string content, HtmlAgilityPack.HtmlNode data)
		{
			string ret = "";
			if (content == "CreateTag") {
				string tagName = data.GetAttributeValue("tagName", null).DecodeXMLString();
				string tagIdStr = data.GetAttributeValue("tagIdStr", null).DecodeXMLString();
				int parentTagId = data.GetAttributeValue("parentTagId", -1);
				int tagId = CreateTag(tagName, tagIdStr, parentTagId);
				TagInfoBase tag = GetTagInfo(tagId);
				if (tag != null)
					return tag.ToXML();
			}
			else if (content == "DeleteTag") {
				int tagId = data.GetAttributeValue("tagId", -1);
				DeleteTag(tagId);
			}
			else if (content == "RenameTag") {
				int tagId = data.GetAttributeValue("tagId", -1);
				string tagName = data.GetAttributeValue("tagName", null).DecodeXMLString();
				RenameTag(tagId, tagName);
			}
			else if (content == "AssignTag") {
				int tagId = data.GetAttributeValue("tagId", -1);
				string itemIds = data.GetAttributeValue("itemIds", "");
				IEnumerable<int> itemIdList = from item in itemIds.Split(",".ToCharArray(), StringSplitOptions.RemoveEmptyEntries) select int.Parse(item);
				MinerSetTagForItems(tagId, itemIdList);
			}
			else if (content == "RemoveTag") {
				int tagId = data.GetAttributeValue("tagId", -1);
				string itemIds = data.GetAttributeValue("itemIds", "");
				IEnumerable<int> itemIdList = from item in itemIds.Split(",".ToCharArray(), StringSplitOptions.RemoveEmptyEntries) select int.Parse(item);
				MinerRemoveTagForItems(tagId, itemIdList);
			}
			return ret;
		}

		public string ExecuteCommand(string command, HtmlAgilityPack.HtmlNode commandNode)
		{
			if (command == "ClearIndex")
				ClearIndex();

			return "";
		}
		#endregion

		#region helper functions
		private void StoreToFile(string postfix, string content)
		{
			try 
			{
				string filename = Path.Combine(SettingsServer.StoreToFileFolder, DateTime.Now.Ticks.ToString() + "-" + postfix);
				File.WriteAllText(filename, content);
			}
			catch { }
		}

		#endregion
	}

}
