using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.IO;
using System.Text.RegularExpressions;
using ContextifyServer.Base;
using Contextify.Shared.Types;
using Contextify.Shared.Interfaces;

namespace ContextifyServer.Base
{
	public partial class PersonInfo: IPersonInfo
	{
		public int PersonId { get; private set; }
		public string Name { get; private set; }
		public EPersonNameTrust NameTrust { get; private set; }	
		private List<int> _accountIds = new List<int>();

		public string CustomData { get; set; }
		
		public static int InvalidPersonId { get { return -9999; } }

		public PersonInfo(MailData mailData, string name, EPersonNameTrust nameTrust)
		{
			_mailData = mailData;
			_peopleData = mailData.PeopleData;
			PersonId = _peopleData.SQLAddPerson();
			SetName(name, nameTrust);
		}

		public PersonInfo(MailData mailData, DataRow data)
		{
			_mailData = mailData;
			_peopleData = mailData.PeopleData;

			try
			{
				PersonId = (int)(long)data[0];
				Name = "";
				if (!System.DBNull.Value.Equals(data[1]))
					Name = (string)data[1];
				else
					GenLib.Log.LogService.LogError("PersonInfo constructor error: Name is null.");
				NameTrust = (EPersonNameTrust)(byte)data[2];
				if (!System.DBNull.Value.Equals(data[3]))
					_accountIds = new List<int>(from id in ((string)data[3]).Split(new string[] { accountSep }, StringSplitOptions.RemoveEmptyEntries) select int.Parse(id));
				if (!System.DBNull.Value.Equals(data[4])) CustomData = (string)data[4];
			}
			catch (Exception ex)
			{
				GenLib.Log.LogService.LogException("PersonInfo constructor: ", ex);
			}
		}

		public PersonInfo(MailData mailData, PersonInfoBase personInfo)
		{
			_mailData = mailData;
			_peopleData = mailData.PeopleData;

			try
			{
				PersonId = personInfo.PersonId;
				Name = personInfo.Name;
				NameTrust = (EPersonNameTrust)personInfo.NameTrust;
				_accountIds = new List<int>(from id in (personInfo.AccountIdsStr).Split(new string[] { accountSep }, StringSplitOptions.RemoveEmptyEntries) select int.Parse(id));
				CustomData = personInfo.CustomData;
			}
			catch (Exception ex)
			{
				GenLib.Log.LogService.LogException("PersonInfo constructor exception: ", ex);
			}
		}

		private static string accountSep = ",";

		public override string ToString() { return Name; }
		
		public bool HasAccountId(int accountId) { return _accountIds != null && _accountIds.Contains(accountId); }
		public IEnumerable<int> GetAccountIds() { return _accountIds.ToList(); }
		public IEnumerable<int> GetEmailIds() { return (from id in _accountIds where _peopleData.GetAccountType(id) == EAccountType.Email select id); }
		public IEnumerable<string> GetEmails()
		{
			return (from id in GetEmailIds() select _peopleData.GetAccount(id)).ToList();
		}

		public string GetAccountIdsStr() { return (_accountIds.Count == 0) ? "" : String.Join(accountSep, _accountIds); }

		private PeopleData _peopleData { get; set; }
		private MailData _mailData { get; set; }
		public PeopleData GetPeopleData() { return _peopleData; }
				
		public void AddAccount(int accountId, bool save = true)
		{
			Debug.Assert(accountId != AccountInfo.InvalidAccountId);
			if (!_accountIds.Contains(accountId))
			{
				_accountIds.Add(accountId);
				if (save)
					_peopleData.SQLUpdatePersonData(PersonId, "AccountIds", GetAccountIdsStr());
			}
		}

		public void RemoveAccount(int accountId, bool save = true)
		{
			if (_accountIds.Contains(accountId))
			{
				_accountIds.Remove(accountId);
				if (save)
					_peopleData.SQLUpdatePersonData(PersonId, "AccountIds", GetAccountIdsStr());
			}
		}

		public bool ContainsAccount(int accountId)
		{
			return _accountIds.Contains(accountId);
		}

		public List<string> GetAccounts()
		{
			return (from id in _accountIds select _peopleData.GetAccount(id)).ToList();
		}		

		public bool SetName(string name, EPersonNameTrust nameTrust, bool save = true)
		{
			if (String.IsNullOrEmpty(name)) return false;
			if (nameTrust == this.NameTrust && name == Name) return false;	
			if (nameTrust >= this.NameTrust)
			{
				Name = name;
				NameTrust = nameTrust;
				if (save)
					SaveContact();
			}
			return true;
		}

		public void SetCustomData(string data)
		{
			CustomData = data;
			_peopleData.SQLUpdatePersonData(PersonId, "CustomData", data);
		}

		internal void CopyDataFromContact(PersonInfo fromPerson)
		{
			if (fromPerson.NameTrust > this.NameTrust)
				this.SetName(fromPerson.Name, fromPerson.NameTrust);
			else if (fromPerson.NameTrust == this.NameTrust)
			{
				int fromNameParts = fromPerson.Name.Split(new char[] { ' ' }).Length;
				int toNameParts = this.Name.Split(new char[] { ' ' }).Length;
				if (fromNameParts > toNameParts)
					this.SetName(fromPerson.Name, fromPerson.NameTrust);
			}
			SaveContact();
		}

		public void SaveContact()
		{
			_peopleData.SQLUpdatePersonData(PersonId, Name, (int)NameTrust, GetAccountIdsStr(), CustomData);
		}

		public string FirstName
		{
			get
			{
				string[] fromParts = Name.Split(new char[] { ' ' });
				if (fromParts.Count() > 0) return fromParts[0];
				else return Name;
			}
		}

		public PersonInfoBase GetPersonInfoBase()
		{
			return new PersonInfoBase() { AccountIdsStr = GetAccountIdsStr(), CustomData = CustomData, Name = Name, NameTrust = (short)NameTrust };
		}

		public string GetPersonXML()
		{
			return GetPersonInfoBase().ToXML();
		}

		public string GetPersonAndAccountsXML()
		{
			string ret = GetPersonInfoBase().ToXML();
			foreach (int accountId in GetAccountIds())
				ret += _mailData.PeopleData.GetAccountInfo(accountId).ToXML();
			return ret;
		}
	}

	public class AccountInfo
	{
		public static int InvalidAccountId { get { return -9999; } }

		public MailData MailData { get; private set; }
		public PeopleData PeopleData { get { return MailData.PeopleData; } }
		public int AccountId { get; private set; }
		public string Account { get { return PeopleData.GetAccount(AccountId); } }
		public string PersonName { get; private set; }
		public string MetaData { get; private set; }

		public AccountInfo(MailData mailData, int accountId)
		{
			MailData = mailData;
			AccountId = accountId;
		}

		public IPersonInfo PersonInfo { get { return PeopleData.GetPersonFromAccount(AccountId); } }

		public override string ToString() { return PersonName + "(" + Account + ")"; }
	}
}
