using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Contextify.Shared.Types;
//using System.Windows.Media;

namespace Contextify.Shared.Interfaces
{
	public interface IPersonInfo
	{
		string FirstName { get; }
		string Name { get; }
		EPersonNameTrust NameTrust { get; }		// return how much do we trust the Name property (how certain we are that it is a valid name)

		IEnumerable<int> GetEmailIds();
		IEnumerable<string> GetEmails();
		IEnumerable<int> GetAccountIds();
		bool ContainsAccount(int accountId);

		string CustomData { get; set; }
		int PersonId { get; }
	}
}
