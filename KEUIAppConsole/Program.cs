using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using KEUIApp;

namespace KEUIAppConsole
{
	class Program
	{
		static void Main(string[] args)
		{
			Console.TreatControlCAsInput = true;

			string dataFolderName = "KEUIApp";
			if (args.Count() > 0)
				dataFolderName = args[0];
			KEUIDialog keui = new KEUIDialog(dataFolderName);
			keui.Init();
			keui.AddEvent("KEUI initialization finished. We are now ready to process events.");
			ConsoleKeyInfo cki;
			while (true)
			{
				cki = Console.ReadKey();
				Console.WriteLine();
				//if (!string.IsNullOrEmpty(line) && line.ToLower() == "exit")
				if ((cki.Modifiers & ConsoleModifiers.Control) != 0 && cki.Key.ToString().ToLower() == "c")
				{
					keui.AddEvent("KEUI is shutting down...");
					break;
				}
				keui.AddEvent("Unknown command. The only valid command is CTRL + C and it will close the application");
			}

			keui.Finish();

		}
	}
}
