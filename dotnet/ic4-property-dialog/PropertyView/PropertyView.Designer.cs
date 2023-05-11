using Microsoft.Win32;
using System.Drawing;
using System.Windows.Forms;

namespace ic4.Examples
{
    partial class PropertyView
    {
        static readonly int width = (int)(480.0f * WinformsUtil.Scaling);
        static readonly int height = height = (int)(800.0f * WinformsUtil.Scaling);
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        private System.Windows.Forms.ComboBox cboVisibility;
        private System.Windows.Forms.Panel panelHeader;
        private System.Windows.Forms.TextBox txtFilter;
        private System.Windows.Forms.Label lblFilter;
        private System.Windows.Forms.Label lblVisibility;

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            //int width = (int)(480.0f * WinformsUtil.Scaling);
           // int height = (int)(800.0f * WinformsUtil.Scaling);

            propertyTree_ = new PropertyTree();
            this.splitContainer_ = new System.Windows.Forms.SplitContainer();
            panel1_ = new Panel();
            this.cboVisibility = new System.Windows.Forms.ComboBox();
            this.panelHeader = new System.Windows.Forms.Panel();
            this.lblVisibility = new System.Windows.Forms.Label();
            this.lblFilter = new System.Windows.Forms.Label();
            this.txtFilter = new System.Windows.Forms.TextBox();
            this.txtDescription_ = new RichTextBox();

            this.panelHeader.SuspendLayout();
            this.SuspendLayout();

            // 
            // panelHeader
            // 
            this.panelHeader.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
           | System.Windows.Forms.AnchorStyles.Right)));
            this.panelHeader.Controls.Add(this.txtFilter);
            this.panelHeader.Controls.Add(this.lblFilter);
            this.panelHeader.Controls.Add(this.lblVisibility);
            this.panelHeader.Controls.Add(this.cboVisibility);
            this.panelHeader.Location = new System.Drawing.Point(0,0);
            this.panelHeader.Size = new System.Drawing.Size(width, Appearance.HeaderRowHeight);
            this.panelHeader.TabIndex = 1;
            this.panelHeader.BackColor = SystemColors.ControlLight;

            int headerCenterY = Appearance.HeaderRowHeight / 2;
            // 
            // lblVisibility
            // 
            this.lblVisibility.Font = new System.Drawing.Font("Microsoft Sans Serif",
            Appearance.ControlFontSize,
            System.Drawing.FontStyle.Regular,
            System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblVisibility.Text = "Visibility:";
            this.lblVisibility.TabIndex = 1;
            this.lblVisibility.AutoSize = true;
            this.lblVisibility.Update();
            this.lblVisibility.Location = new System.Drawing.Point(Appearance.TextSpacing / 2, headerCenterY - this.lblVisibility.Height/2);

            int headerCtrlHeight = (int)(21.0f * WinformsUtil.Scaling);

            // 
            // cboVisibility
            // 
            this.cboVisibility.Font = new System.Drawing.Font("Microsoft Sans Serif",
              Appearance.ControlFontSize,
              System.Drawing.FontStyle.Regular,
              System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.cboVisibility.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cboVisibility.FormattingEnabled = true;
            this.cboVisibility.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.cboVisibility.Size = new System.Drawing.Size((int)(120 * WinformsUtil.Scaling), headerCtrlHeight);
            this.cboVisibility.Location = new System.Drawing.Point(
                lblVisibility.Location.X + lblVisibility.Width + Appearance.TextSpacing / 4,
                headerCenterY - this.cboVisibility.Height / 2);
            this.cboVisibility.TabIndex = 0;
            this.cboVisibility.Items.AddRange(new object[] { "Beginner", "Expert", "Guru" });
            this.cboVisibility.SelectedIndex = 0;
            this.cboVisibility.SelectedIndexChanged += CboVisibility_SelectedIndexChanged;
            // 
            // lblFilter
            // 
            this.lblFilter.Font = new System.Drawing.Font("Microsoft Sans Serif",
             Appearance.ControlFontSize,
             System.Drawing.FontStyle.Regular,
             System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblFilter.TabIndex = 2;
            this.lblFilter.Text = "Filter:";
            this.lblFilter.AutoSize = true;
            this.lblFilter.Update();
            this.lblFilter.Location = new System.Drawing.Point(
               cboVisibility.Location.X + cboVisibility.Width + Appearance.TextSpacing, 
               headerCenterY - this.lblFilter.Height / 2);

            // 
            // txtFilter
            // 
            var txtFilterLocation = lblFilter.Location.X + lblFilter.Width + Appearance.TextSpacing / 4;
            this.txtFilter.Font = new System.Drawing.Font("Microsoft Sans Serif",
              Appearance.ControlFontSize,
              System.Drawing.FontStyle.Regular,
              System.Drawing.GraphicsUnit.Point, ((byte)(0)));
           
            this.txtFilter.Size = new System.Drawing.Size(
                width - txtFilterLocation - Appearance.TextSpacing, headerCtrlHeight);
            this.txtFilter.Location = new System.Drawing.Point(txtFilterLocation, headerCenterY - this.txtFilter.Height / 2);
            this.txtFilter.TabIndex = 3;
            this.txtFilter.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top)
             | System.Windows.Forms.AnchorStyles.Left)
             | System.Windows.Forms.AnchorStyles.Right)));
            this.txtFilter.TextChanged += TxtFilter_TextChanged;

            // 
            // splitContainer1
            // 
            this.splitContainer_.Size = new System.Drawing.Size(width, height-Appearance.HeaderRowHeight - (int)(2 * WinformsUtil.Scaling));
            this.splitContainer_.Location = new Point(0, Appearance.HeaderRowHeight + (int)(2 * WinformsUtil.Scaling));
            this.splitContainer_.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top)
              | System.Windows.Forms.AnchorStyles.Left)
              | System.Windows.Forms.AnchorStyles.Right
              | System.Windows.Forms.AnchorStyles.Bottom)));
            this.splitContainer_.Orientation = System.Windows.Forms.Orientation.Horizontal;
            this.splitContainer_.SplitterDistance = height/4*3;
            this.splitContainer_.SplitterWidth = (int)(4.0f * WinformsUtil.Scaling);
            //this.splitContainer_.Panel1.AutoScroll = true;

            propertyTreePanel_ = new CustomPanel();
            propertyTreePanel_.Parent = this.splitContainer_.Panel1;
            propertyTreePanel_.Dock = DockStyle.Fill;
            propertyTreePanel_.AutoScroll = true;

            // 
            // panel1
            // 
            this.panel1_.BackColor = System.Drawing.SystemColors.ControlLight;
            this.panel1_.Dock = DockStyle.Fill;
            this.panel1_.Parent = this.splitContainer_.Panel2;
            this.panel1_.BorderStyle = BorderStyle.None;
            this.panel1_.Padding = new Padding((int)(8.0f * WinformsUtil.Scaling));
            
            // 
            // txtDescription_
            // 
            this.txtDescription_.BackColor = System.Drawing.SystemColors.ControlLight;
            this.txtDescription_.Dock = DockStyle.Fill;
            this.txtDescription_.Parent = this.panel1_;
            this.txtDescription_.BorderStyle = BorderStyle.None;
            this.txtDescription_.Font = new System.Drawing.Font("Microsoft Sans Serif", 
                Appearance.InfoBoxFontSize,
                System.Drawing.FontStyle.Regular,
                System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            //
            // PropertyTree
            //
            propertyTree_.Parent = propertyTreePanel_;// this.splitContainer_.Panel1;
            propertyTree_.Size = splitContainer_.Panel1.ClientSize;
            propertyTree_.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top)
               | System.Windows.Forms.AnchorStyles.Left)
               | System.Windows.Forms.AnchorStyles.Right)));

            // 
            // Ic4PropertyDialog
            // 
            this.ClientSize = new System.Drawing.Size(width, height);
            this.Controls.Add(this.splitContainer_);
            this.Controls.Add(this.panelHeader);
            this.BackColor = SystemColors.ControlDark;
           
            this.panelHeader.ResumeLayout(false);
            this.panelHeader.PerformLayout();
            this.ResumeLayout(false);

        }

        private RichTextBox txtDescription_;
        private System.Windows.Forms.Panel panel1_;
        private CustomPanel propertyTreePanel_;

        #endregion
    }
}