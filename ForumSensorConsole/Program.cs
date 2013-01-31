using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using ForumSensor;

namespace ForumSensorConsole
{
	class Program
	{
		static void Main(string[] args)
		{
			string dataFolderName = "ForumSensor";
			if (args.Count() > 0)
				dataFolderName = args[0];

			Console.TreatControlCAsInput = true;
			ForumSensorDialog forumSensor = new ForumSensorDialog(dataFolderName);

			forumSensor.Init();
			Console.WriteLine("ForumSensor initialization finished.");
			forumSensor.ButtonStart_Click(null, null);
			
			ConsoleKeyInfo cki;
			while (true)
			{
				cki = Console.ReadKey();
				Console.WriteLine();
				//if (!string.IsNullOrEmpty(line) && line.ToLower() == "exit")
				if ((cki.Modifiers & ConsoleModifiers.Control) != 0 && cki.Key.ToString().ToLower() == "c")
				{
					Console.WriteLine("Forum Sensor is shutting down...");
					break;
				}
				Console.WriteLine("Unknown command. The only valid command is CTRL + C and it will close the application");
			}

			forumSensor.ButtonStart_Click(null, null);
			Console.WriteLine("Finishing downloading of posts in the queue. Please wait...");
			if (forumSensor.WebGetter != null)
			{
				while (forumSensor.WebGetter.ActiveQueueSize > 0)
					System.Threading.Thread.Sleep(1000);
			}
			// add extra few seconds to process all pages
			System.Threading.Thread.Sleep(3000);
			forumSensor.Finish();

			//Console.WriteLine("Sensor closed. Press any key to close the window.");
			//Console.ReadKey();
		}
	}
}
