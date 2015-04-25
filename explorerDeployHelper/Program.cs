using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;

namespace explorerDeployHelper
{
    class Program
    {
        static void KillExplorer()
        {
            Process[] expProcs = Process.GetProcessesByName("explorer");

            foreach (Process proc in expProcs)
                proc.Kill();
        }

        static bool ExplorerExists()
        {
            bool alive;

            Process[] procs = Process.GetProcessesByName("explorer");

            alive = procs.Length > 0;

            procs = null;
            return alive;
        }

        static void StartExplorer()
        {
            if (!ExplorerExists())
            {
                ProcessStartInfo psi = new ProcessStartInfo();
                psi.UseShellExecute = true;
                psi.FileName = "C:\\Windows\\explorer.exe";
                psi.WorkingDirectory = "C:\\Windows\\System32";

                Process.Start(psi);
            }
        }

        static void Main(string[] args)
        {
            if (args.Length > 0)
            {
                switch (args[0])
                {
                    case "-kill":
                        KillExplorer();
                        break;
                    case "-start":
                        StartExplorer();
                        break;
                }
            }

            Environment.Exit(0);
        }
    }
}
