using System;
using System.Diagnostics;
using System.Collections;
using System.Collections.Generic;
using System.Data;
using ContextifyServer.Base;
using System.Linq;
using Contextify.Shared.Types;

namespace ContextifyServer.Base
{
	public class MailDataSettings
	{			
		protected object _lockObj = new object();
		public DateTime InvalidDate = new DateTime(1,1,1);

		protected GenFiles.SQLiteDatabase sqlData = null;

		public const int SQLStarredTagId = 0;
		
		public MailDataSettings(string dbFolder)
		{
			try
			{
				sqlData = new GenFiles.SQLiteDatabase("Data Source=" + System.IO.Path.Combine(dbFolder, "itemData.db3") + ";Pooling=true;Journal Mode=Persist;MaxPageCount=100;");
				sqlData.ExecuteNonQuery("PRAGMA synchronous = OFF;");
				//SQLCreateTables();
			}
			catch (System.Exception ex)
			{
				//MessageBox.Show("Error in MailData constructor: " + ex.Message); 
				GenLib.Log.LogService.LogException("Error in MailData constructor: ", ex);
			}
		}

		public virtual void Dispose()
		{
			if (sqlData != null)
				sqlData.Dispose();
			sqlData = null;
		}

		public bool SQLiteDllValid()
		{
			return sqlData != null;
		}

		protected void SQLClearItems()
		{
			sqlData.ExecuteNonQuery("DROP TABLE IF EXISTS Items;");
			SQLCreateItemsTable();
		}

		protected void SQLClearTags()
		{
			sqlData.ExecuteNonQuery("DROP TABLE IF EXISTS Tags;");
			SQLCreateTagsTable();
		}

		protected void SQLClearSettings()
		{
			sqlData.ExecuteNonQuery("DROP TABLE IF EXISTS Settings;");
			sqlData.ExecuteNonQuery("CREATE TABLE IF NOT EXISTS Settings (Key TEXT UNIQUE NOT NULL PRIMARY KEY, Value TEXT);");
		}
		
		private void SQLCreateItemsTable()
		{
			sqlData.ExecuteNonQuery("CREATE TABLE IF NOT EXISTS Items (ItemId INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, ItemType TINYINT DEFAULT 0, AccountId INTEGER, EntryId TEXT, ItemState TINYINT DEFAULT 0, Subject VARCHAR, Body VARCHAR, MetaData VARCHAR);");
			sqlData.ExecuteNonQuery("CREATE UNIQUE INDEX IF NOT EXISTS Items_Index ON Items (EntryId ASC);");
		}

		private void SQLCreateTagsTable()
		{
			sqlData.ExecuteNonQuery("CREATE TABLE IF NOT EXISTS TagInfo (TagId INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, TagName TEXT, TagIdStr TEXT, TagMeta TEXT, TagParentId INTEGER NOT NULL)");
			sqlData.ExecuteNonQuery(String.Format("INSERT OR IGNORE INTO TagInfo (TagId, TagName, TagIdStr, TagParentId) VALUES ({0}, 'Emails', 'Root--Emails', {1})", MailData.TagIdEmails, MailData.TagIdRoot));
			sqlData.ExecuteNonQuery(String.Format("INSERT OR IGNORE INTO TagInfo (TagId, TagName, TagIdStr, TagParentId) VALUES ({0}, 'Attachments', 'Root--Attachments', {1})", MailData.TagIdAttachments, MailData.TagIdRoot));
			sqlData.ExecuteNonQuery(String.Format("INSERT OR IGNORE INTO TagInfo (TagId, TagName, TagIdStr, TagParentId) VALUES ({0}, 'Documents', 'Root--Documents', {1})", MailData.TagIdDocuments, MailData.TagIdRoot));

			sqlData.ExecuteNonQuery(String.Format("INSERT OR IGNORE INTO TagInfo (TagId, TagName, TagIdStr, TagParentId) VALUES ({0}, 'Social Networks', 'Root--Social Networks', {1})", MailData.TagIdSocialNetworks, MailData.TagIdRoot));
			sqlData.ExecuteNonQuery(String.Format("INSERT OR IGNORE INTO TagInfo (TagId, TagName, TagIdStr, TagParentId) VALUES ({0}, 'Facebook', 'Facebook', {1})", MailData.TagIdFacebook, MailData.TagIdSocialNetworks));
			sqlData.ExecuteNonQuery(String.Format("INSERT OR IGNORE INTO TagInfo (TagId, TagName, TagIdStr, TagParentId) VALUES ({0}, 'LinkedIn', 'LinkedIn', {1})", MailData.TagIdLinkedIn, MailData.TagIdSocialNetworks));
			sqlData.ExecuteNonQuery(String.Format("INSERT OR IGNORE INTO TagInfo (TagId, TagName, TagIdStr, TagParentId) VALUES ({0}, 'Twitter', 'Twitter', {1})", MailData.TagIdTwitter, MailData.TagIdSocialNetworks));

			sqlData.ExecuteNonQuery(String.Format("INSERT OR IGNORE INTO TagInfo (TagId, TagName, TagIdStr, TagParentId) VALUES ({0}, 'Custom Tags', 'Root--Custom Tags', {1})", MailData.TagIdCustomTags, MailData.TagIdRoot));
			sqlData.ExecuteNonQuery(String.Format("INSERT OR IGNORE INTO TagInfo (TagId, TagName, TagIdStr, TagParentId) VALUES ({0}, 'Starred', 'Starred', {1})", MailData.TagIdStarred, MailData.TagIdCustomTags));
			sqlData.ExecuteNonQuery(String.Format("INSERT OR IGNORE INTO TagInfo (TagId, TagName, TagIdStr, TagParentId) VALUES ({0}, 'Relevant', 'Relevant', {1})", MailData.TagIdRelevant, MailData.TagIdCustomTags));
			sqlData.ExecuteNonQuery(String.Format("INSERT OR IGNORE INTO TagInfo (TagId, TagName, TagIdStr, TagParentId) VALUES ({0}, 'Later', 'Later', {1})", MailData.TagIdLater, MailData.TagIdCustomTags));
			sqlData.ExecuteNonQuery(String.Format("INSERT OR IGNORE INTO TagInfo (TagId, TagName, TagIdStr, TagParentId) VALUES ({0}, 'Hide', 'Hide', {1})", MailData.TagIdHide, MailData.TagIdCustomTags));
		}

		#region Settings table functions
		
		public DateTime SQLLastClearIndexTime
		{
			get
			{
				var ret = sqlData.ExecuteScalar("SELECT Value FROM Settings WHERE Key='LastClearIndexTime'");
				if (ret != null)
					return DateTime.Parse((string) ret);
				else
					return InvalidDate;
			}
			protected set
			{
				sqlData.ExecuteNonQuery(String.Format("REPLACE INTO Settings (Key, Value) VALUES ('LastClearIndexTime', '{0}')", value.ToString()));
			}
		}

		public bool SQLFullIndexingCompleted
		{
			get 
			{
				var ret = sqlData.ExecuteScalar("SELECT Value FROM Settings WHERE Key='FullIndexingCompleted'");
				if (ret != null)
					return (string)ret == "1";
				else
					return false;
			}
			set
			{
				sqlData.ExecuteNonQuery(String.Format("REPLACE INTO Settings (Key, Value) VALUES ('FullIndexingCompleted', '{0}')", value == false ? 0 : 1));
			}
		}
		#endregion

		#region Item table functions

		public int SQLGetLastItemRowId()
		{
			return (int)(long)sqlData.ExecuteScalar("SELECT MAX(rowid) FROM Items");	
		}

		public void SQLAddItem(string entryId, ESQLItemType type, int accountId = 0, ESQLItemState itemState = ESQLItemState.Indexed)
		{
			var command = sqlData.CreateCommand(String.Format("INSERT OR IGNORE INTO Items (EntryId) VALUES (@EntryId); UPDATE Items SET AccountId={0}, ItemType={1} WHERE EntryId=@EntryId;", accountId, (int)type));
			command.Parameters.AddWithValue("@EntryId", entryId);
			sqlData.ExecuteNonQuery(command);
			sqlData.DisposeCommand(command);
		}

		public bool SQLIsItemId(int itemId)
		{
			return SQLGetItem(itemId) != null;
		}
		
		public IEnumerable<int> SQLGetItemIds(int accountId)
		{
			DataTable table = sqlData.GetDataTable(String.Format("SELECT ItemId FROM Items WHERE AccountId={0}", accountId));
			return from DataRow row in table.Rows select (int)(long)row[0];
		}
		
		public DataTable SQLGetItems()
		{
			DataTable table = sqlData.GetDataTable(String.Format("SELECT * FROM Items"));
			return table;
		}

		public DataRow SQLGetItem(int itemId)
		{
			DataTable table = sqlData.GetDataTable(String.Format("SELECT * FROM Items WHERE ItemId={0}", itemId));
			if (table.Rows.Count > 0)
				return table.Rows[0];
			else
				return null;
		}

		public DataRow SQLGetItem(string entryId)
		{
			var command = sqlData.CreateCommand("SELECT * FROM Items WHERE EntryId=@EntryId");
			command.Parameters.AddWithValue("@EntryId", entryId);
			DataTable table = sqlData.GetDataTable(command);
			sqlData.DisposeCommand(command);
			if (table.Rows.Count > 0)
				return table.Rows[0];
			else
				return null;
		}

		// return the next unprocessed element and remove it from the queue
		public ESQLItemState SQLGetItemState(int itemId)
		{
			var command = sqlData.CreateCommand("SELECT ItemState FROM Items WHERE ItemId=@ItemId");
			command.Parameters.AddWithValue("@ItemId", itemId);
			var ret = sqlData.ExecuteScalar(command);
			sqlData.DisposeCommand(command);
			if (ret != null)
				return (ESQLItemState)(byte)ret;
			else
				return ESQLItemState.NotIndexed;
		}

		// return the next unprocessed element and remove it from the queue
		public ESQLItemState SQLGetItemState(string entryId)
		{
			var command = sqlData.CreateCommand("SELECT ItemState FROM Items WHERE EntryId=@EntryId");
			command.Parameters.AddWithValue("@EntryId", entryId);
			var ret = sqlData.ExecuteScalar(command);
			sqlData.DisposeCommand(command);
			if (ret != null)
				return (ESQLItemState)(byte)ret;
			else
				return ESQLItemState.NotIndexed;
		}

		public void SQLSetItemState(int itemId, ESQLItemState newState)
		{
			sqlData.ExecuteNonQuery(String.Format("UPDATE Items SET ItemState = {0} WHERE ItemId={1}", (int)newState, itemId));
		}


		public Tuple<string,string, string> SQLGetItemSubjectBodyMeta(int itemId)
		{
			DataTable table = sqlData.GetDataTable(String.Format("SELECT Subject, Body, MetaData FROM Items WHERE ItemId={0}", itemId));
			return new Tuple<string, string, string>((string)table.Rows[0][0], (string)table.Rows[0][1], (string)table.Rows[0][2]);
		}

		public string SQLGetItemBody(int itemId)
		{
			return (string) sqlData.ExecuteScalar(String.Format("SELECT Body FROM Items WHERE ItemId={0}", itemId));
		}

		public string SQLGetItemMetaData(int itemId)
		{
			return (string)sqlData.ExecuteScalar(String.Format("SELECT MetaData FROM Items WHERE ItemId={0}", itemId));
		}

		public void SQLSetItemStateForFolderId(int folderId, ESQLItemState newState)
		{
			sqlData.ExecuteNonQuery(String.Format("UPDATE Items SET ItemState = {1} WHERE FolderId={0}", folderId, (int)newState));
		}

		public void SQLSetItemSubjectBodyMeta(int itemId, string subject, string body, string metaData)
		{
			Debug.Assert(SQLIsItemId(itemId), "Unable to set data for invalid itemId");
			var command = sqlData.CreateCommand("UPDATE Items SET Subject=@Subject, Body=@Body, MetaData=@MetaData WHERE ItemId=" + itemId.ToString());
			command.Parameters.AddWithValue("@Subject", subject);
			command.Parameters.AddWithValue("@Body", body);
			command.Parameters.AddWithValue("@MetaData", metaData);
			var ret = sqlData.ExecuteNonQuery(command);
			sqlData.DisposeCommand(command);
		}

		public void SQLSetItemBody(int itemId, string body)
		{
			Debug.Assert(SQLIsItemId(itemId), "Unable to set body for invalid itemId");
			var command = sqlData.CreateCommand("UPDATE Items SET Body=@Body WHERE ItemId=" + itemId.ToString());
			command.Parameters.AddWithValue("@Body", body);
			var ret = sqlData.ExecuteNonQuery(command);
			sqlData.DisposeCommand(command);
		}

		public void SQLSetItemMeta(int itemId, string meta)
		{
			Debug.Assert(SQLIsItemId(itemId), "Unable to set metadata for invalid itemId");
			var command = sqlData.CreateCommand("UPDATE Items SET MetaData=@MetaData WHERE ItemId=" + itemId.ToString());
			command.Parameters.AddWithValue("@MetaData", meta);
			var ret = sqlData.ExecuteNonQuery(command);
			sqlData.DisposeCommand(command);
		}
		
		public void SQLRemoveItemFromTable(string entryId)
		{
			var command = sqlData.CreateCommand("DELETE FROM Items WHERE EntryId = @EntryId");
			command.Parameters.AddWithValue("@EntryId", entryId);
			sqlData.ExecuteNonQuery(command);
			sqlData.DisposeCommand(command);
		}

		public void SQLRemoveItemFromTable(int itemId)
		{
			var command = sqlData.CreateCommand("DELETE FROM Items WHERE ItemId = @ItemId");
			command.Parameters.AddWithValue("@ItemId", itemId);
			sqlData.ExecuteNonQuery(command);
			sqlData.DisposeCommand(command);
		}

		public int SQLGetItemId(string entryId)
		{
			var command = sqlData.CreateCommand("SELECT ItemId FROM Items WHERE EntryId=@EntryId");
			command.Parameters.AddWithValue("@EntryId", entryId);
			var ret = sqlData.ExecuteScalar(command);
			sqlData.DisposeCommand(command);
			if (ret != null)
				return (int)(long)ret;
			else
				return -1;
		}

		public string SQLGetEntryId(int itemId)
		{
			var ret = sqlData.ExecuteScalar(String.Format("SELECT EntryId FROM Items WHERE ItemId = {0}", itemId));
			if (ret != null)
				return (string) ret;
			else
				return null;
		}

		public void SQLRemoveItem(int itemId)
		{
			sqlData.ExecuteNonQuery(String.Format("DELETE FROM Items WHERE ItemId = {0}", itemId));
		}

		public void SQLRemoveItems(IEnumerable<int> itemIdList)
		{
			var command = sqlData.CreateCommand("DELETE FROM Items WHERE ItemId = @ItemId");

			using (var trans = sqlData.BeginTransaction())
			{
				foreach (int itemId in itemIdList)
				{
					command.Parameters.Clear();
					command.Parameters.AddWithValue("@ItemId", itemId);
					command.ExecuteNonQuery();
				}
				trans.Commit();
			}
			sqlData.DisposeCommand(command);
		}

		public void SQLRemoveItem(string entryId)
		{
			var command = sqlData.CreateCommand("DELETE FROM Items WHERE EntryId = @EntryId");
			command.Parameters.AddWithValue("@EntryId", entryId);
			sqlData.ExecuteNonQuery(command);
			sqlData.DisposeCommand(command);
		}

		public void SQLRemoveItems(IEnumerable<string> entryIdList)
		{
			var command = sqlData.CreateCommand("DELETE FROM Items WHERE EntryId = @EntryId");

			using (var trans = sqlData.BeginTransaction())
			{
				foreach (string entity in entryIdList)
				{
					command.Parameters.Clear();
					command.Parameters.AddWithValue("@EntryId", entity);
					command.ExecuteNonQuery();
				}
				trans.Commit();
			}
			sqlData.DisposeCommand(command);
		}

		#endregion

		#region tag functions
		public int SQLGetTagCount()
		{
			var ret = sqlData.ExecuteScalar("SELECT COUNT(*) FROM TagInfo");
			if (ret != null)
				return (int)(long)ret;
			else
				return 0;
		}

		public IEnumerable<TagInfoBase> SQLGetTagInfos(int offset = -1, int count = -1)
		{
			DataTable table = null;
			if (count == -1 && offset == -1)
				table = sqlData.GetDataTable("SELECT * FROM TagInfo");
			else
				table = sqlData.GetDataTable(String.Format("SELECT * FROM TagInfo LIMIT {0}, {1}", offset, count));
			return from DataRow row in table.Rows select new TagInfoBase((string)row[1], (int)(long)row[0], (string)row[2], (int)(long)row[4], row[3] is DBNull ? null : (string)row[3]);
			
		}

		public int SQLGetTagId(string tagName, int tagParentId)
		{
			var command = sqlData.CreateCommand("SELECT TagId FROM TagInfo WHERE TagName = @TagName AND TagParentId = @TagParentId");
			command.Parameters.AddWithValue("@TagName", tagName);
			command.Parameters.AddWithValue("@TagParentId", tagParentId);
			var ret = sqlData.ExecuteScalar(command);
			sqlData.DisposeCommand(command);
			if (ret == null) return -1;
			return (int)(long)ret;
		}

		//protected void SQLTagItemIdList(IEnumerable<int> itemIdList, int tagId)
		//{
		//    using (SQLiteTransaction trans = sqlData.BeginTransaction())
		//    {
		//        foreach (int itemId in itemIdList)
		//            sqlData.ExecuteNonQuery(String.Format("INSERT OR IGNORE INTO Tags (ItemId, TagId) VALUES ({0}, {1})", itemId, tagId));
		//        trans.Commit();
		//    }		
		//}

		//public void SQLTagItemId(int itemId, int tagId)
		//{
		//    sqlData.ExecuteNonQuery(String.Format("INSERT OR IGNORE INTO Tags (ItemId, TagId) VALUES ({0}, {1})", itemId, tagId));
		//}

		//public void SQLUntagItemId(int itemId, int tagId)
		//{
		//    sqlData.ExecuteNonQuery(String.Format("DELETE FROM Tags WHERE ItemId = {0} AND TagId = {1}", itemId, tagId));
		//}

		//public bool SQLIsItemIdTagged(int itemId, int tagId)
		//{
		//    var ret = sqlData.ExecuteScalar(String.Format("SELECT count(*) FROM Tags WHERE TagId={0} AND ItemId = {1}", tagId, itemId));
		//    if (ret != null)
		//        return (int)(long)ret > 0;
		//    else
		//        return false;
		//}

		protected int SQLCreateTag(string tagName, string tagIdStr, int tagParentId, string tagMeta)
		{
			var command = sqlData.CreateCommand("INSERT INTO TagInfo (TagName, TagIdStr, TagParentId, TagMeta) VALUES (@TagName, @TagIdStr, @TagParentId, @TagMeta)");
			command.Parameters.AddWithValue("@TagName", tagName);
			command.Parameters.AddWithValue("@TagIdStr", tagIdStr);
			command.Parameters.AddWithValue("@TagParentId", tagParentId);
			command.Parameters.AddWithValue("@TagMeta", tagMeta);
			sqlData.ExecuteNonQuery(command);
			sqlData.DisposeCommand(command);
			return (int)(long)sqlData.ExecuteScalar("SELECT last_insert_rowid();");	// return the id of the inserted tag
		}

		protected void SQLRenameTag(int tagId, string tagName)
		{
			var command = sqlData.CreateCommand("UPDATE TagInfo SET TagName=@TagName WHERE TagId=@TagId");
			command.Parameters.AddWithValue("@TagName", tagName);
			command.Parameters.AddWithValue("@TagId", tagId);
			sqlData.ExecuteNonQuery(command);
			sqlData.DisposeCommand(command);
		}

		public void SQLDeleteTag(int tagId)
		{
			sqlData.ExecuteNonQuery(String.Format("DELETE FROM TagInfo WHERE TagId={0}", tagId));
			//sqlData.ExecuteNonQuery(String.Format("DELETE FROM Tags WHERE TagId={0}", tagId));
			// todo: if this tag has children the we have to recursively delete also the children
		}
		#endregion

		#region table compatibility functions
		protected bool SQLTablesCompatible()
		{
			return SQLIsItemsTableCompatible() && SQLIsTagsTableCompatible();
		}

		/// <summary>
		/// Check if the structure of existing databases is correct. If not, recreate tables
		/// </summary>
		/// <returns>true if tables were ok, false if they had to be recreated</returns>
		protected bool SQLRecreateTablesIfNotCompatible()
		{
			if (SQLTablesCompatible())
				return true;
			if (!SQLIsSettingsTableCompatible())
				SQLClearSettings();
			if (!SQLIsItemsTableCompatible())
				SQLClearItems();
			if (!SQLIsTagsTableCompatible())
				SQLClearTags();
			return false;
		}

		private bool SQLIsItemsTableCompatible()
		{
			return sqlData.CheckTableColumns("Items", new List<string>() { 
						"ItemId", "INTEGER", 
						"ItemType", "TINYINT", 
						"AccountId", "INTEGER", 
						"EntryId", "TEXT", 
						"ItemState", "TINYINT",
						"Subject", "VARCHAR", 
						"Body", "VARCHAR",
						"MetaData", "VARCHAR"});
		}

		private bool SQLIsSettingsTableCompatible()
		{
			return sqlData.CheckTableColumns("Settings", new List<string>() { "Key", "TEXT", "Value", "TEXT" });
		}

		private bool SQLIsTagsTableCompatible()
		{
			return sqlData.CheckTableColumns("TagInfo", new List<string>() {
				"TagId", "INTEGER", 
				"TagName", "TEXT", 
				"TagIdStr", "TEXT", 
				"TagMeta", "TEXT", 
				"TagParentId", "INTEGER" });
		}
		#endregion
	}
		

	

}
