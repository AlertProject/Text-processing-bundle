using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;
using System.Xml.XPath;

namespace Contextify.Shared.Types
{
#if! SILVERLIGHT
	[Serializable()]
#endif
	public class GroupInfoBase
	{
		public string GroupName;
		public List<int> PersonIdList;

		public override string ToString()
		{
			return String.Format("{0} ({1})", GroupName, PersonIdList);
		}
	}
}
