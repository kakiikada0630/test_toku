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

        [DllImport("DebgApl.dll")]
        static extern void OpenSerial(string name);
        [DllImport("DebgApl.dll")]
        static extern void FileOpen(string f_name);

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
        }

        private void button1_Click(object sender, EventArgs e)
        {
            string name = "COM7";
            string file_name = "AAAA";
            OpenSerial(name);
            FileOpen(file_name);

        }

        private void button2_Click(object sender, EventArgs e)
        {

        }
    }
}
