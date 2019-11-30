using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.Text;


namespace WindowsFormsApp1
{
    public partial class Form1 : Form
    {
        private int counter;

        [DllImport("DebgApl.dll")]
        static extern void OpenSerial(string name);
        [DllImport("DebgApl.dll")]
        static extern void FileOpen(string f_name);
        [DllImport("DebgApl.dll")]
        static extern void WriteCmd(string f_name);
        [DllImport("DebgApl.dll")]
        static extern void CloseSerial();
        [DllImport("DebgApl.dll")]
        static extern void FileClose();

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            counter = 0;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            string name      = "COM7";
            string file_name = "file_"+counter.ToString();
            OpenSerial(name);
            FileOpen(file_name);
            WriteCmd("ig1 on\n");
            WriteCmd("vbu on\n");
            counter++;
        }

        private void button2_Click(object sender, EventArgs e)
        {
            WriteCmd("vbu off\n");
            WriteCmd("ig1 off\n");
            FileClose();
            CloseSerial();
        }
    }
}
