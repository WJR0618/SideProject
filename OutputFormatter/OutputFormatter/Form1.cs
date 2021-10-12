using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace OutputFormatter
{
    public partial class PAL測資小幫手 : Form
    {
        public PAL測資小幫手()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            string str = textBox1.Text;
            int start = 0;
            int end = str.Length;
            try
            {
                start = str.IndexOf("正確的輸出應該是>>");
                end = str.IndexOf("你的程式的輸出是>>");
                str = str.Substring(start, end - start - 3);
                str = str.Replace("\\", "\\\\");
                str = str.Replace("\"", "\\\"");
                str = str.Replace("\'", "\\\'");
                string[] strArr = str.Split(new string[] { "\r\n" }, StringSplitOptions.None);

                strArr[0] = strArr[0].Replace("正確的輸出應該是>>", "");
                strArr[strArr.Length - 1] = strArr[strArr.Length - 1].Replace("<<", "");

                string newStr = "";

                foreach (string s in strArr)
                {
                    newStr += "    cout << \"" + s + "\\n\" ;\r\n";
                } // foreach

                newStr = newStr.Substring(0, newStr.Length - 8);
                newStr += "\" ;";
                textBox2.Text = newStr;
            }
            catch
            {
                textBox2.Text = textBox1.Text;
            }
            
            
            
        }

        private void button2_Click(object sender, EventArgs e)
        {
            textBox2.Text = "";
            textBox1.Text = "請輸入PAL之Output";
        }
    }
}
