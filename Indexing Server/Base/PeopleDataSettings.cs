using System;
using System.Diagnostics;
using System.Collections;
using System.Collections.Generic;
using System.Data;
using ContextifyServer.Base;
using System.Linq;

namespace ContextifyServer.Base
{
	public class PeopleDataSettings
	{
		private object _sqlLockObj = new object();
		protected GenFiles.SQLiteDatabase sqlData = null;

		public class FromToInfo
		{
			public int ToCount;
			public int FromCount;
			public void AddFromCount(int count) { FromCount += count;  }
			public void AddToCount(int count) { ToCount += count; }
		}

		public enum TotalsColumn { FromTotal = 0, ToTotal };

	 
		public PeopleDataSettings(string dbFolder)
		{
			try
			{
				if (!System.IO.Directory.Exists(dbFolder))
					System.IO.Directory.CreateDirectory(dbFolder);

				sqlData = new GenFiles.SQLiteDatabase("Data Source=" + System.IO.Path.Combine(dbFolder, "peopleData.db3") + ";Pooling=true;Journal Mode=Persist;MaxPageCount=100;");
				sqlData.ExecuteNonQuery("PRAGMA synchronous = OFF;");
				SQLCreateTables();
			}
			catch (System.Exception ex)
			{
				//MessageBox.Show("Error in PeopleData constructor: " + ex.Message); 
				GenLib.Log.LogService.LogException("Error in PeopleData constructor: ", ex);
			}
		}

		public virtual void Dispose()
		{
			if (sqlData != null)
				sqlData.Dispose();
			sqlData = null;
		}

		#region Account table functions

		protected int SQLAddNewAccount(string account, int accountType, int personId, string personName, string metaData = "")
		{
			var command = sqlData.CreateCommand("INSERT INTO Accounts (Account, AccountType, PersonId, PersonName, MetaData) VALUES (@Account, @AccountType, @PersonId, @PersonName, @MetaData)");
			command.Parameters.AddWithValue("@Account", account);
			command.Parameters.AddWithValue("@AccountType", (int) accountType);
			command.Parameters.AddWithValue("@PersonId", personId);
			command.Parameters.AddWithValue("@PersonName", personName);
			command.Parameters.AddWithValue("@MetaData", metaData);
			sqlData.ExecuteNonQuery(command);
			sqlData.DisposeCommand(command);
			return (int)(long)sqlData.ExecuteScalar("SELECT last_insert_rowid();");
		}

		public void SQLSetAccountMetaData(int accountId, string metaData)
		{
			var command = sqlData.CreateCommand(String.Format("UPDATE Accounts SET MetaData = @paramVal WHERE AccountId = {0}", accountId));
			command.Parameters.AddWithValue("@paramVal", metaData);
			sqlData.ExecuteNonQuery(command);
			sqlData.DisposeCommand(command);
		}

		protected DataTable SQLGetAccounts()
		{
			return sqlData.GetDataTable("SELECT AccountId, Account, AccountType, PersonId, PersonName, MetaData FROM Accounts");
		}

		protected void SQLSetPersonId(int accountId, int personId)
		{
			sqlData.ExecuteNonQuery(String.Format("UPDATE Accounts SET PersonId = {1} WHERE AccountId = {0};", accountId, personId));
		}

		protected void SQLIncreaseTotalCount(int accountId, TotalsColumn column)
		{
			int old = SQLGetTotalCount(accountId, column);
			sqlData.ExecuteNonQuery(String.Format("INSERT OR IGNORE INTO Accounts (AccountId) VALUES ({0}); UPDATE Accounts SET {1} = {2} WHERE AccountId = {0};", accountId, column == TotalsColumn.FromTotal ? "FromTotal" : "ToTotal", old + 1));
		}

		public int SQLGetTotalCount(int accountId, TotalsColumn column)
		{
			var ret = sqlData.ExecuteScalar(String.Format("SELECT {0} FROM Accounts WHERE AccountId='{1}'", column == TotalsColumn.FromTotal ? "FromTotal" : "ToTotal", accountId));
			if (ret != null)
				return (int)(long)ret;
			return 0;
		}

		public int SQLGetTotalCount(IEnumerable<int> accountIdList, TotalsColumn column)
		{
			DataTable table = sqlData.GetDataTable(String.Format("SELECT {0} FROM Accounts WHERE AccountId IN ({1})", column == TotalsColumn.FromTotal ? "FromTotal" : "ToTotal", String.Join(",", from id in accountIdList select id.ToString())));
			IEnumerable<int> vals = from DataRow row in table.Rows select (int)(long)row[0];
			return vals.Sum();
		}
		
		// get from and to counts between emailId and other emails
		protected Dictionary<int, FromToInfo> SQLGetAccountIdTotals()
		{
			Dictionary<int, FromToInfo> info = new Dictionary<int, FromToInfo>();
			DataTable table = sqlData.GetDataTable("SELECT AccountId, FromTotal, ToTotal FROM Accounts");
			foreach (DataRow row in table.Rows)
				info[(int)(long)row[0]] = new FromToInfo() { FromCount = (int)(long)row[1], ToCount = (int)(long)row[2] };
			return info;
		}
		#endregion

		#region FromToCounts functions
		protected int SQLGetFromToCount(int accountId1, int accountId2)
		{
			var ret = sqlData.ExecuteScalar(String.Format("SELECT Id1ToId2 FROM FromToCounts WHERE AccountId1 = {0} AND AccountId2 = {1}", Math.Min(accountId1, accountId2), Math.Max(accountId1, accountId2)));
			if (ret != null)
				return (int)(long)ret;
			return 0;
		}

		protected void SQLIncreaseFromToCount(int accountId1, int accountId2)
		{
			int old = SQLGetFromToCount(accountId1, accountId2);
			string column = accountId1 < accountId2 ? "Id1ToId2" : "Id2ToId1";
			sqlData.ExecuteNonQuery(String.Format("INSERT OR IGNORE INTO FromToCounts (AccountId1, AccountId2) VALUES ({0}, {1}); UPDATE FromToCounts SET {2} = {3} WHERE AccountId1 = {0} AND AccountId2 = {1};", Math.Min(accountId1, accountId2), Math.Max(accountId1, accountId2), column, old + 1));
		}

		// get from and to counts between emailId and other emails
		public Dictionary<int, FromToInfo> SQLGetFromToTotals(int accountId)
		{
			Dictionary<int, FromToInfo> info = new Dictionary<int, FromToInfo>();
			DataTable table = sqlData.GetDataTable(String.Format("SELECT * FROM FromToCounts WHERE AccountId1={0} OR AccountId2={0}", accountId));

			foreach (DataRow row in table.Rows)
			{
				if (accountId == (int)(long)row[0])
					info[(int)(long)row[1]] = new FromToInfo() { ToCount = (int)(long)row[2], FromCount = (int)(long)row[3] };
				else
					info[(int)(long)row[1]] = new FromToInfo() { FromCount = (int)(long)row[2], ToCount = (int)(long)row[3] };
			}
			return info;
		}

		public Dictionary<int, FromToInfo> SQLGetFromToTotals(IEnumerable<int> accountIdList)
		{
			Dictionary<int, FromToInfo> info = new Dictionary<int, FromToInfo>();
			if (accountIdList == null || accountIdList.Count() == 0)
				return info;
			DataTable table = sqlData.GetDataTable(String.Format("SELECT * FROM FromToCounts WHERE AccountId1 IN ({0}) OR AccountId2 IN ({0})", String.Join(",", from id in accountIdList select id.ToString())));
			foreach (DataRow row in table.Rows)
			{
				int id1 = (int)(long)row[0];
				int id2 = (int)(long)row[1];
				int id1ToId2 = (int)(long)row[2];
				int id2ToId1 = (int)(long)row[3];
				if (accountIdList.Contains(id1))
					SQLAddToDict(info, id2, id2ToId1, id1ToId2);
				if (accountIdList.Contains(id2))
					SQLAddToDict(info, id1, id1ToId2, id2ToId1);
			}
			return info;
		}

		public Dictionary<string, int> SQLGetFromToCounts(IEnumerable<int> accountIdList)
		{
			HashSet<int> accountIdHash = new HashSet<int>(accountIdList);
			Dictionary<string, int> info = new Dictionary<string, int>();
			DataTable table = sqlData.GetDataTable(String.Format("SELECT * FROM FromToCounts WHERE AccountId1 IN ({0}) OR AccountId2 IN ({0})", String.Join(",", from id in accountIdList select id.ToString())));
			foreach (DataRow row in table.Rows)
			{
				int id1 = (int)(long)row[0];
				int id2 = (int)(long)row[1];
				if (accountIdHash.Contains(id1) && accountIdHash.Contains(id2))
				{
					info[String.Format("{0}-{1}", id1, id2)] = (int)(long)row[2];
					info[String.Format("{0}-{1}", id2, id1)] = (int)(long)row[3];
				}
			}
			return info;
		}

		// increase the counts in the dictionary 
		private void SQLAddToDict(Dictionary<int, FromToInfo> dict, int key, int fromCount, int toCount)
		{
			if (!dict.ContainsKey(key))
				dict[key] = new FromToInfo();
			dict[key].FromCount += fromCount;
			dict[key].ToCount += toCount;
		}
		#endregion

		#region Settings table functions
		public int SQLGetSettingInt(string key, int defaultValue)
		{
			string data = SQLGetSetting(key);
			if (data == null)
				return defaultValue;
			return int.Parse(data);
		}

		public String SQLGetSetting(string key)
		{
			var command = sqlData.CreateCommand("SELECT Value FROM Settings WHERE Key = @paramVal");
			command.Parameters.AddWithValue("@paramVal", key);

			var ret = sqlData.ExecuteScalar(command);
			sqlData.DisposeCommand(command);
			if (ret == null)
				return null;
			return (string) ret;
		}

		public void SQLSetSetting(string key, int value)
		{
			SQLSetSetting(key, value.ToString());
		}

		public void SQLSetSetting(string key, string value)
		{
			sqlData.ExecuteNonQuery(String.Format("INSERT OR IGNORE INTO Settings (Key) VALUES ('{0}'); UPDATE Settings SET Value = '{1}' WHERE Key='{0}';", key, value));
		}

		protected void SQLClearPotentialOwnerAccountIds()
		{
			SQLSetSetting("PotentialOwners", "");
		}

		protected List<int> SQLGetPotentialOwnerAccountIds()
		{
			try 
			{	        
				string data = SQLGetSetting("PotentialOwners");
				if (string.IsNullOrEmpty(data))
					return new List<int>();
				return new List<int>(from s in data.Split(",".ToCharArray(), StringSplitOptions.RemoveEmptyEntries) select int.Parse(s));
			}
			catch (Exception)
			{
				return new List<int>();
			}
		}
		#endregion

		#region People table functions
		public int SQLAddPerson()
		{
			sqlData.ExecuteNonQuery("INSERT INTO People (Name) VALUES ('')");
			return (int)(long)sqlData.ExecuteScalar("SELECT last_insert_rowid();");
		}

		public void SQLRemovePerson(int personId)
		{
			sqlData.ExecuteNonQuery(String.Format("DELETE FROM People WHERE Id = {0}", personId));
		}

		// set data for a person
		// NOTE: The person MUST already exist in the table
		public void SQLUpdatePersonData(int personId, string column, string data)
		{
			var command = sqlData.CreateCommand(String.Format("UPDATE People SET {1} = @paramVal WHERE Id = {0}", personId, column));
			command.Parameters.AddWithValue("@paramVal", data);
			sqlData.ExecuteNonQuery(command);
			sqlData.DisposeCommand(command);
		}

		public void SQLUpdatePersonData(int personId, string column, int data)
		{
			sqlData.ExecuteNonQuery(String.Format("UPDATE People SET {1} = {2} WHERE Id = {0}", personId, column, data));
		}

		// update all data for the given contact
		public void SQLUpdatePersonData(int personId, string name, int nameTrust, string accountIdsStr, string customData)
		{
			var command = sqlData.CreateCommand(String.Format("UPDATE People SET NameTrust={0}, Name=@Name, AccountIds=@AccountIds, CustomData=@CustomData WHERE Id = {1} ", nameTrust, personId));
			command.Parameters.AddWithValue("@Name", name);
			command.Parameters.AddWithValue("@AccountIds", accountIdsStr);
			command.Parameters.AddWithValue("@CustomData", customData);
			sqlData.ExecuteNonQuery(command);
			sqlData.DisposeCommand(command);
		}

		public DataTable SQLGetPeopleData()
		{
			return sqlData.GetDataTable("SELECT * FROM People");
		}
		#endregion
		
		#region Creating/clearing tables
		public void SQLClearCounts()
		{
			SQLClearEmailsCountsTable();
			sqlData.ExecuteNonQuery("DROP TABLE IF EXISTS FromToCounts;");
			SQLCreateTables();
		}

		protected void SQLClearTables()
		{
			sqlData.ExecuteNonQuery("DROP TABLE IF EXISTS Accounts;");
			sqlData.ExecuteNonQuery("DROP TABLE IF EXISTS FromToCounts;");
			
			sqlData.ExecuteNonQuery("DROP TABLE IF EXISTS People;");
			sqlData.ExecuteNonQuery("DROP TABLE IF EXISTS Settings;");
			
			SQLCreateTables();
			SQLInitSettingsTable();
		}

		private void SQLClearEmailsCountsTable()
		{
			sqlData.ExecuteNonQuery("UPDATE Accounts SET FromTotal = 0");
			sqlData.ExecuteNonQuery("UPDATE Accounts SET ToTotal = 0");
		}

		protected void SQLInitSettingsTable()
		{
			SQLSetSetting("OwnerAccountId", -1);
			SQLSetSetting("PotentialOwners", "");
			//sqlData.ExecuteNonQuery(String.Format("INSERT OR IGNORE INTO Settings (Key) VALUES ('{0}'); UPDATE Settings SET Value = {1} WHERE Key = '{0}'", "Version", Helper.GetCurrentAssemblyVersion()));
		}

		protected void SQLCreateTables()
		{
			sqlData.ExecuteNonQuery("CREATE TABLE IF NOT EXISTS Accounts (AccountId INTEGER PRIMARY KEY NOT NULL, Account TEXT NOT NULL, AccountType TINYINT NOT NULL, PersonId INTEGER NOT NULL, PersonName TEXT NULL, MetaData TEXT NULL, FromTotal INTEGER DEFAULT 0, ToTotal INTEGER DEFAULT 0)");
			sqlData.ExecuteNonQuery("CREATE TABLE IF NOT EXISTS FromToCounts (AccountId1 INTEGER NOT NULL, AccountId2 INTEGER NOT NULL, Id1ToId2 INTEGER DEFAULT 0, Id2ToId1 INTEGER DEFAULT 0);");

			sqlData.ExecuteNonQuery("CREATE TABLE IF NOT EXISTS People (Id INTEGER NOT NULL PRIMARY KEY, Name TEXT NULL, NameTrust TINYINT DEFAULT 1, AccountIds TEXT NULL, CustomData TEXT NULL)");
			sqlData.ExecuteNonQuery("CREATE TABLE IF NOT EXISTS Settings (Key TEXT UNIQUE NOT NULL PRIMARY KEY, Value TEXT)");

			sqlData.ExecuteNonQuery("CREATE UNIQUE INDEX IF NOT EXISTS FromToCounts_Index ON FromToCounts (AccountId1 ASC, AccountId2 ASC);");
			sqlData.ExecuteNonQuery("CREATE UNIQUE INDEX IF NOT EXISTS Accounts_Index ON Accounts (Account ASC, AccountType ASC);");
		}

		// this function checks if the existing tables have the expected structure
		// use this at the beginning to see if we need to delete existing data and restart indexing
		protected bool SQLTablesCompatible()
		{
			bool okAccounts = sqlData.CheckTableColumns("Accounts", new List<string>() { 
				"AccountId", "INTEGER", 
				"Account", "TEXT", 
				"AccountType", "TINYINT", 
				"PersonId", "INTEGER", 
				"PersonName", "TEXT", 
				"MetaData", "TEXT", 
				"FromTotal", "INTEGER", 
				"ToTotal", "INTEGER"});
			bool okFromToCounts = sqlData.CheckTableColumns("FromToCounts", new List<string>() { 
				"AccountId1", "INTEGER", 
				"AccountId2", "INTEGER", 
				"Id1ToId2", "INTEGER", 
				"Id2ToId1", "INTEGER" });
			bool okPeople = sqlData.CheckTableColumns("People", new List<string>() { 
				"Id", "INTEGER", 
				"Name", "TEXT", 
				"NameTrust", "TINYINT", 
				"AccountIds", "TEXT", 
				"CustomData", "TEXT" });
			bool okSettings = sqlData.CheckTableColumns("Settings", new List<string>() { "Key", "TEXT", "Value", "TEXT" });

			return (okAccounts && okFromToCounts && okPeople && okSettings);
		}
		#endregion
	}
}
