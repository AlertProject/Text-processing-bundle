using System;
using System.Data;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;
using System.IO;

using System.Globalization;
using System.Text.RegularExpressions;
using System.Threading;
using ContextifyServer.Base;
using Contextify.Shared.Types;
using GenLib.Text;
using HtmlAgilityPack;

namespace ContextifyServer.Base
{
	public partial class PeopleData : PeopleDataSettings
	{
		Dictionary<string, int> _accountToAccountId = new Dictionary<string, int>();
		Dictionary<int, string> _accountIdToAccount = new Dictionary<int, string>();
		private Dictionary<int, int> _accountIdToPersonId = new Dictionary<int, int>();
		private Dictionary<int, PersonInfo> _personIdToPerson = new Dictionary<int, PersonInfo>();
		private Dictionary<int, string> _accountIdToUUID = new Dictionary<int, string>();
		private Dictionary<int, string> _accountIdToPersonName = new Dictionary<int, string>();
		//private Dictionary<string, List<int>> _UUIDToAccountIdList = new Dictionary<string, List<int>>();
		
		private object _lockObj = new object();
		private object _lockMergingObj = new object();
		
		public MailData MailData { get; private set; }
		public SettingsServer Settings { get; private set; }

		public PeopleData(MailData mailData) : base(mailData.Settings.ProfileFolder)
		{
			MailData = mailData;
			Settings = mailData.Settings;
		}

		public bool LoadData()
		{
			DateTime start = DateTime.Now;

			lock (_lockObj)
			{
				try
				{
					if (!SQLTablesCompatible())
					{
						SQLClearTables();
						return false;
					}

					// get info about all accounts
					DataTable table = SQLGetAccounts();
					string accountKey;
					int accountId;
					string account;
					string personName;
					string metaData;
					foreach (DataRow row in table.Rows)
					{
						try
						{
							accountId = (int)(long)row[0];
							account = (string)row[1];
							EAccountType accountType = (EAccountType)(byte)row[2];
							int personId = (int)(long)row[3];
							personName = (string)row[4];
							metaData = (string)row[5];
							Debug.Assert(!_accountIdToAccount.ContainsKey(accountId), String.Format("Account id {0} is not unique", accountId));		// account id has to be unique
							Debug.Assert(!_accountToAccountId.ContainsKey(account), String.Format("Account name {0} is not unique", account));			// account has to be unique
							Debug.Assert(accountId >= 0, "Account id is negative");
							Debug.Assert(!string.IsNullOrEmpty(account), String.Format("Account for id {0} is empty or null", accountId));					// we should not allow empty accounts
							accountKey = GetAccountKey(account, accountType);
							_accountIdToAccount[accountId] = accountKey;
							_accountToAccountId[accountKey] = accountId;
							_accountIdToPersonId[accountId] = personId;
							_accountIdToPersonName[accountId] = personName;
							_accountIdToUUID[accountId] = metaData;
							//if (!string.IsNullOrEmpty(metaData)) {
							//	if (!_UUIDToAccountIdList.ContainsKey(metaData))
							//		_UUIDToAccountIdList[metaData] = new List<int>();
							//	_UUIDToAccountIdList[metaData].Add(accountId);
							//}
						}
						catch (Exception e)
						{
							GenLib.Log.LogService.LogException("Failed to load a data row from EmailAddressed table. ", e);
						}
					}
					
					
					table = SQLGetPeopleData();
					foreach (DataRow row in table.Rows)
					{
						PersonInfo person = new PersonInfo(MailData, row);
						_personIdToPerson[person.PersonId] = person;
					}
				}
				catch (System.Exception ex)
				{
					GenLib.Log.LogService.LogException("Failed to load contact information.", ex);
					return false;
				}
			}

			GenLib.Log.LogService.LogInfo(String.Format("--- Loading people data needed {0:f0} miliseconds ---", (DateTime.Now - start).TotalMilliseconds));
			return true;
		}
	
		public void DeletePeopleInfo()
		{
			lock (_lockObj)
			{
				GenLib.Log.LogService.LogInfo("Clearing people database...");
				SQLClearTables();

				_accountIdToPersonId = new Dictionary<int, int>();
				_accountIdToAccount = new Dictionary<int, string>();
				_personIdToPerson = new Dictionary<int, PersonInfo>();
				_accountIdToUUID = new Dictionary<int, string>();
			}
		}
						
		public IEnumerable<PersonInfo> GetPeopleList()
		{
			return _personIdToPerson.Values.AsEnumerable<PersonInfo>();		// we call ToList() so that we create a new list with the objects. otherwise we might get the collection changed" exception
		}
		
		public PersonInfo GetEmptyPersonInfo()
		{
			return new PersonInfo(MailData, new PersonInfoBase());
		}

		public int GetInvalidAccountId() { return AccountInfo.InvalidAccountId; }
		public int GetInvalidPersonId() { return PersonInfo.InvalidPersonId; }

		#region account functions
		
		public int GetAccountId(string account, EAccountType accountType)
		{
			try
			{
				if (String.IsNullOrEmpty(account))
					return AccountInfo.InvalidAccountId;
				string accountKey = GetAccountKey(account, accountType);
				if (_accountToAccountId.ContainsKey(accountKey))
					return _accountToAccountId[accountKey];
				return AccountInfo.InvalidAccountId;
			}
			catch (Exception ex)
			{
				GenLib.Log.LogService.LogException("Exception in GetAccountId: ", ex);
				return AccountInfo.InvalidAccountId;
			}
		}

		public IEnumerable<int> GetAccountIds()
		{
			return _accountIdToAccount.Keys.AsEnumerable<int>();
		}

		public string GetPersonNameForAccount(int accountId)
		{
			if (_accountIdToPersonName.ContainsKey(accountId))
				return _accountIdToPersonName[accountId];
			return "";
		}

		public bool IsKnownAccount(string account, EAccountType accountType)
		{
			return GetAccountId(account, accountType) != AccountInfo.InvalidAccountId;
		}

		public int GetAccountCount()
		{
			return _accountIdToAccount.Count;
		}

		public string GetAccountKey(string account, EAccountType accountType)
		{
			return ((char)accountType).ToString() + account;
		}

		public string GetAccount(int accountId)
		{
			try
			{
				if (_accountIdToAccount.ContainsKey(accountId))
					return _accountIdToAccount[accountId].Substring(1);
			}
			catch (Exception ex) { GenLib.Log.LogService.LogException("Exception in GetAccount: ", ex); }
			return null;
		}

		public string GetAccountUUID(int accountId)
		{
			try
			{
				if (_accountIdToUUID.ContainsKey(accountId))
					return _accountIdToUUID[accountId];
			}
			catch (Exception ex) { GenLib.Log.LogService.LogException("Exception in GetAccountUUID: ", ex); }
			return null;
		}

		public void SetAccountUUID(int accountId, string UUID)
		{
			try {
				// remove the account id from the old UUID -> account list
				//if (_accountIdToUUID.ContainsKey(accountId)) {
				//	string oldUUID = _accountIdToUUID[accountId];
				//	if (_UUIDToAccountIdList.ContainsKey(oldUUID) && _UUIDToAccountIdList[oldUUID].Contains(accountId))
				//		_UUIDToAccountIdList[oldUUID].Remove(accountId);
				//}
				// assign the account id to new uuid
				_accountIdToUUID[accountId] = UUID;
				//if (!_UUIDToAccountIdList.ContainsKey(UUID))
				//	_UUIDToAccountIdList[UUID] = new List<int>();
				//// add the account id to the list of accounts associated with the given uuid
				//_UUIDToAccountIdList[UUID].Add(accountId);
				SQLSetAccountMetaData(accountId, UUID);
			}
			catch (Exception ex) { GenLib.Log.LogService.LogException("Exception in SetAccountUUID: ", ex); }
		}


		public EAccountType GetAccountType(int accountId)
		{
			try
			{
				if (_accountIdToAccount.ContainsKey(accountId))
					return (EAccountType)_accountIdToAccount[accountId][0];
			}
			catch (Exception ex) { GenLib.Log.LogService.LogException("Exception in GetAccountType: ", ex); }
			return EAccountType.Invalid;
		}

		public IEnumerable<int> GetAccountsByType(EAccountType type)
		{
			return (from accountId in _accountIdToAccount.Keys where (EAccountType)_accountIdToAccount[accountId][0] == type select accountId).ToList();		// return a copy of the list so that we don't get the "collection changed" exception
		}

		public int GetEmailFromToCount(int emailIdFrom, int emailIdTo)
		{
			if (emailIdFrom == AccountInfo.InvalidAccountId || emailIdTo == AccountInfo.InvalidAccountId)
				return 0;
			return SQLGetFromToCount(emailIdFrom, emailIdTo);
		}

		public int GetEmailToCount(int emailId)
		{
			if (emailId == AccountInfo.InvalidAccountId) return 0;
			return SQLGetTotalCount(emailId, TotalsColumn.ToTotal);
		}

		public int GetEmailFromCount(int emailId)
		{
			if (emailId == AccountInfo.InvalidAccountId) return 0;
			return SQLGetTotalCount(emailId, TotalsColumn.FromTotal);
		}
		#endregion

		#region person functions
		
		public PersonInfo GetPerson(int personId)
		{
			try
			{
				if (_personIdToPerson.ContainsKey(personId))
					return _personIdToPerson[personId];
				return null;
			}
			catch (Exception ex)
			{
				// we might get an exception if another thread deletes the person from the dictionary immediately after our ContainsKey return true
				GenLib.Log.LogService.LogException("Exception while accessing the _personIdToPerson: ", ex);
				return null;
			}
		}

		public PersonInfo GetPerson(string account, EAccountType accountType)
		{
			int id = GetPersonId(account, accountType);
			if (id == PersonInfo.InvalidPersonId)
				return null;
			return GetPerson(id);
		}

		public PersonInfo GetPersonFromAccount(int accountId)
		{
			int id = GetPersonId(accountId);
			if (id == PersonInfo.InvalidPersonId)
				return null;
			return GetPerson(id);
		}

		public bool TryGetPerson(string account, EAccountType accountType, out PersonInfo person)
		{
			person = GetPerson(account, accountType);
			return person != null;
		}

		public int GetPersonId(string account, EAccountType accountType)
		{
			int accountId = GetAccountId(account, accountType);
			return GetPersonId(accountId);
		}

		public int GetPersonId(int accountId)
		{
			if (accountId == AccountInfo.InvalidAccountId)
				return PersonInfo.InvalidPersonId;
			if (!_accountIdToPersonId.ContainsKey(accountId))
				return PersonInfo.InvalidPersonId;
			return _accountIdToPersonId[accountId];
		}

		public string GetPersonName(int personId)
		{
			PersonInfo person = GetPerson(personId);
			if (person != null) 
				return person.Name;
			return "";
		}

		public string GetPersonName(string account, EAccountType accountType)
		{
			PersonInfo info = GetPerson(account, accountType);
			if (info != null)
				return info.Name;
			return "";
		}

		public int GetPersonTotalToCount(PersonInfo person)
		{
			if (person == null) return 0;
			int tot = 0;
			foreach (int id in person.GetAccountIds())
				tot += GetEmailToCount(id);
			return tot;
		}

		public int GetPersonTotalFromCount(PersonInfo person)
		{
			if (person == null) return 0;
			int tot = 0;
			foreach (int id in person.GetAccountIds())
				tot += GetEmailFromCount(id);
			return tot;
		}

		public int GetPersonTotalFromToCount(PersonInfo fromPerson, PersonInfo toPerson)
		{
			if (fromPerson == null) return 0;
			if (toPerson == null) return 0;
			int tot = 0;
			foreach (int e1 in fromPerson.GetAccountIds())
				foreach (int e2 in toPerson.GetAccountIds())
					tot += GetEmailFromToCount(e1, e2);
			return tot;
		}

		public void AssignAccount(string account, EAccountType accountType, PersonInfo person)
		{
			lock (_lockObj)
			{
				Debug.Assert(person != null);
				
				// create a new account				
				Debug.Assert(!IsKnownAccount(account, accountType));
				int accountId = SQLAddNewAccount(account, (int)accountType, person.PersonId, person.Name);
				string accountKey = GetAccountKey(account, accountType);
				_accountToAccountId[accountKey] = accountId;
				_accountIdToAccount[accountId] = accountKey;
				_accountIdToPersonName[accountId] = person.Name;

				person.AddAccount(accountId);
				_accountIdToPersonId[accountId] = person.PersonId;		// use the same id as was set for the first email of the person
				SQLSetPersonId(accountId, person.PersonId);
			}
		}

		public void MergePersons(PersonInfo fromPerson, PersonInfo toPerson)
		{
			Debug.Assert(fromPerson != null);
			Debug.Assert(toPerson != null);
			foreach (int id in fromPerson.GetAccountIds())
				ReassignAccountToExistingAccount(id, fromPerson, toPerson);
			lock (_lockObj) {
				toPerson.CopyDataFromContact(fromPerson);
			}
		}

		public void ReassignAccountToExistingAccount(int accountId, PersonInfo fromPerson, PersonInfo toPerson)
		{
			Debug.Assert(fromPerson != null);
			Debug.Assert(toPerson != null);
			if (fromPerson.PersonId == toPerson.PersonId)
				return;
			lock (_lockObj) {
				fromPerson.RemoveAccount(accountId);
				toPerson.AddAccount(accountId);

				_accountIdToPersonId[accountId] = toPerson.PersonId;
				SQLSetPersonId(accountId, toPerson.PersonId);

				// update person info based on account type
				EAccountType accountType = GetAccountType(accountId);
				
				// if the fromPerson has no more email contacts then we remove the reference to the fromPerson
				bool noMoreAccounts = (fromPerson.GetAccountIds().Count() == 0);
				if (noMoreAccounts) {
					_personIdToPerson.Remove(fromPerson.PersonId);
					SQLRemovePerson(fromPerson.PersonId);
				}
			}
		}

		public PersonInfo ReassignAccountToNewAccount(int accountId)
		{
			PersonInfo fromPerson = (PersonInfo)GetPersonFromAccount(accountId);
			Debug.Assert(fromPerson != null);
			PersonInfo newPerson = null;
			lock (_lockObj) {
				newPerson = new PersonInfo(MailData, fromPerson.Name, fromPerson.NameTrust);
				_personIdToPerson[newPerson.PersonId] = newPerson;
			}
			Debug.Assert(newPerson != null);
			ReassignAccountToExistingAccount(accountId, fromPerson, newPerson);
			return newPerson;
		}

		public PersonInfo AddNewPersonOrUpdate(string personName, string account, EAccountType accountType, EPersonNameTrust trust)
		{
			lock (_lockObj)
			{
				PersonInfo person = null;
				if (IsKnownAccount(account, accountType))
				{
					person = GetPerson(account, accountType);
					Debug.Assert(person != null, String.Format("PersonInfo is null. The account {0} does not seem to be associated with a contact.", account));
					person.SetName(personName, trust);
				}
				else
				{
					Debug.Assert(!String.IsNullOrEmpty(personName), String.Format("Invalid name for the contact with account {0}. Empty name is not valid.", account));
					Debug.Assert(!String.IsNullOrEmpty(account), String.Format("Invalid account name for contact {0}. Empty account name is not valid.", personName));
					Debug.Assert(!IsKnownAccount(account, accountType), "The account is already associated with a person.");
					//Trace.WriteLineIf(account.ToLower() != account, String.Format("!!! Warning: added account {0} was not in lower case. Did you forget to put it in lower case?", account));

					person = new PersonInfo(MailData, personName, trust);
					_personIdToPerson[person.PersonId] = person;
					AssignAccount(account, accountType, person);
				}

				return person;
			}
		}
		#endregion
	
		public void IncreaseCountInfo(int fromAccountId, List<int> recipientsAccountIds)
		{
			lock (_lockObj)
			{
				using (var trans = sqlData.BeginTransaction())
				{
					SQLIncreaseTotalCount(fromAccountId, TotalsColumn.FromTotal);
					foreach (int toAccountId in recipientsAccountIds)
					{
						SQLIncreaseFromToCount(fromAccountId, toAccountId);
						SQLIncreaseTotalCount(toAccountId, TotalsColumn.ToTotal);
					}
					trans.Commit();
				}
			}
		}
		
		public int GetPeopleCount()
		{
			return _personIdToPerson.Count();
		}

		public IEnumerable<string> GetPersonNameList()
		{
			lock (_lockObj)
			{
				return (from p in _personIdToPerson.Values select p.Name).AsEnumerable<string>();		// create a list so that we don't get the "collection changed" exception
			}
		}

		public IEnumerable<PersonInfo> GetPeople(bool ordered = false)
		{
			if (ordered == true)
				return _personIdToPerson.Values.OrderBy(p => p.Name).AsEnumerable<PersonInfo>();		// create a copy of the list so that we don't get the "collection changed" exception
			else
				return _personIdToPerson.Values.AsEnumerable<PersonInfo>();
		}

		public IEnumerable<int> GetPeopleIds()
		{
			return _personIdToPerson.Keys.AsEnumerable<int>();
		}

		
		public Dictionary<int, FromToInfo> GetAccountIdTotals()
		{
			return SQLGetAccountIdTotals();
		}

		public Dictionary<int, FromToInfo> GetPersonIdTotals()
		{
			Dictionary<int, FromToInfo> accountIdTotals = GetAccountIdTotals();
			Dictionary<int, FromToInfo> personIdTotals = new Dictionary<int, FromToInfo>();
			foreach (var person in GetPeople())
			{
				FromToInfo info = new FromToInfo();
				foreach (int id in person.GetAccountIds())
				{
					if (accountIdTotals.ContainsKey(id))
					{
						info.FromCount += accountIdTotals[id].FromCount;
						info.ToCount += accountIdTotals[id].ToCount;
					}
				}
				personIdTotals[person.PersonId] = info;
			}
			return personIdTotals;
		}

		public IEnumerable<AccountInfoBase> GetAccountInfoList()
		{
			return from accountId in _accountIdToAccount.Keys select new AccountInfoBase() { Account = GetAccount(accountId), PersonId = GetPersonId(accountId), AccountId = accountId, AccountType = (short) GetAccountType(accountId) };
		}

		public IEnumerable<PersonInfoBase> GetPeopleInfoList()
		{
			return from personInfo in _personIdToPerson.Values select personInfo.GetPersonInfoBase();
		}

		public AccountInfoBase GetAccountInfo(int accountId)
		{
			if (GetAccount(accountId) == null)
				return null;
			return new AccountInfoBase() { Account = GetAccount(accountId), PersonId = GetPersonId(accountId), AccountId = accountId, AccountType = (short)GetAccountType(accountId) };
		}
		
		public string RequestData(string content, HtmlNode dataQueryNode)
		{
			return "";
		}

		public string ExecuteCommand(string command, HtmlAgilityPack.HtmlNode commandNode)
		{
			string ret = "";
			return ret;
		}
	}
}
