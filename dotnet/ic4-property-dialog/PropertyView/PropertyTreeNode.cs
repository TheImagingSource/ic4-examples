using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.Versioning;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Xml.Linq;
using static System.Net.Mime.MediaTypeNames;

namespace ic4.Examples
{
    class PropertyTreeNodeCollectionEventArgs : EventArgs
    {
        public PropertyTreeNode Node { get; internal set; }
    };

    class PropertyTreeNodeCollection : CollectionBase
    {
        private PropertyTreeNode parent_;

        public event EventHandler<PropertyTreeNodeCollectionEventArgs> NodeAdded;
        public event EventHandler<PropertyTreeNodeCollectionEventArgs> NodeRemoved;

        public PropertyTreeNodeCollection(PropertyTreeNode parent)
        {
            parent_ = parent;
        }

        protected override void OnClear()
        {
            List.OfType<PropertyTreeNode>().ToList().ForEach(node =>
            {
                NodeRemoved?.Invoke(this, new PropertyTreeNodeCollectionEventArgs() { Node = node });
            });
            base.OnClear();
        }

        public int Add(PropertyTreeNode node)
        {
            if (node == null)
            {
                throw new ArgumentNullException("node");
            }

            (node as Control).Parent = parent_;
            List.Add(node);
            NodeAdded?.Invoke(this, new PropertyTreeNodeCollectionEventArgs() { Node = node });
            return List.Count - 1;
        }

        public void Remove(PropertyTreeNode node)
        {
            if (List.Contains(node))
            {
                List.Remove(node);
                NodeRemoved?.Invoke(this, new PropertyTreeNodeCollectionEventArgs() { Node = node });
            }
        }
    }

    [ToolboxItem(false)]
    class PropertyTreeNode : Control
    {
        private static Bitmap ExpandImage = null;
        private static Bitmap CollapseImage = null;


        private int rowHeight_ = Appearance.HeaderRowHeight;

        private bool isExpanded_ = true;
        private bool isExpandable_ = true;
        private bool ShowHeader_ = true;
        private Control PropertyTree_ = null;

        private Label text_;
        private PictureBox pictureBox_;
        private Control propertyControl_;
        private PropertyTreeNodeCollection nodes_;

        public event EventHandler NodeSlected;


        public Control PropertyControl  => propertyControl_;

        public bool ShowHeader
        { 
            set
            {
                ShowHeader_ = value;
                text_.Visible = value;
                pictureBox_.Visible = value;
            }
            get
            {
                return ShowHeader_;
            }
        }


        public PropertyTreeNode(string name, Control propertyControl)
        {
            if (ExpandImage == null)
            {
                var assembly = Assembly.GetExecutingAssembly();
                ExpandImage = new Bitmap(assembly.GetManifestResourceStream("PropertyView.images.expand.png"));
                CollapseImage = new Bitmap(assembly.GetManifestResourceStream("PropertyView.images.collapse.png"));
            }

            Name = name;
            propertyControl_ = propertyControl;
            DoubleBuffered = true;
            Initialize();
        }

        public void Updatelayout(int x, int y)
        {
            this.SuspendLayout();
            WinformsUtil.SuspendDrawing(this.Parent);

            int nodeContentHeight = 0;

            if (propertyControl_ != null)
            {
                nodeContentHeight += propertyControl_.Height + Appearance.Spacing;
            }
            else
            {
                int horizontalNodeOffset = 0;
                int verticalNodeOffset =  0;

                if(ShowHeader)
                {
                    horizontalNodeOffset = Appearance.Indentation;
                    verticalNodeOffset = rowHeight_;
                }

                var children = Nodes.OfType<PropertyTreeNode>();
                foreach (var cat in children)
                {
                    cat.Parent = this;
                    cat.Updatelayout(horizontalNodeOffset, verticalNodeOffset);

                    nodeContentHeight += cat.Size.Height + Appearance.Spacing;
                    verticalNodeOffset += (cat.Size.Height + Appearance.Spacing);
                }
            }

            this.Location = new Point(x, y + Appearance.Spacing);

            // size
            var height = rowHeight_;
            if(isExpanded_)
            {
                if (ShowHeader)
                {
                    height = rowHeight_ + nodeContentHeight;
                }
                else
                {
                    height = nodeContentHeight;
                }
            }
            this.Size = new Size(Parent.ClientSize.Width - x, height);

            WinformsUtil.ResumeDrawing(this.Parent);
            this.ResumeLayout();
            this.Refresh();
        }

        private void Initialize()
        {
            this.Size = new Size(300, 0);
            this.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;

            pictureBox_ = new PictureBox()
            {
                Parent = this,
                BackColor = SystemColors.ControlLight,
                BackgroundImage = CollapseImage,
                BackgroundImageLayout = System.Windows.Forms.ImageLayout.Center,
                Location = new System.Drawing.Point(0, 0),
                Size = new System.Drawing.Size(Appearance.Indentation, rowHeight_)
            };

            text_ = new Label()
            {
                Parent = this,
                Location = new System.Drawing.Point(pictureBox_.Width, 0),
                Size = new System.Drawing.Size(this.Width - pictureBox_.Width, rowHeight_),
                Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right,
                Text = Name,
                TextAlign = ContentAlignment.MiddleLeft,
                Font = new System.Drawing.Font("Microsoft Sans Serif", Appearance.NodeFontSize, FontStyle.Regular, GraphicsUnit.Point, 0),
                BackColor = SystemColors.ControlLight
            };

            text_.MouseDown += PropertyCategory_MouseClick;
            pictureBox_.MouseDown += PropertyCategory_MouseClick;
        }

        public PropertyTreeNodeCollection Nodes
        {
            get
            {
                if (nodes_ == null)
                {
                    nodes_ = new PropertyTreeNodeCollection(this);
                    nodes_.NodeAdded += Nodes_NodeAdded;
                    nodes_.NodeRemoved += Nodes__NodeRemoved;
                }
                return nodes_;
            }
        }

        private Control PropertyTree
        {
            get
            {
                if (PropertyTree_ == null)
                {
                    var p = this.Parent;
                    while (p != null)
                    {
                        if (p is PropertyTree)
                        {
                            PropertyTree_ = p;
                            break;
                        }
                        p = p.Parent;
                    }
                }
               
                return PropertyTree_;
            }
        }

        void PropertyCategory_MouseClick(object sender, MouseEventArgs e)
        {
            if (isExpanded_)
            {
                Collapse();
            }
            else
            {
                Expand();
            }

            NodeSlected?.Invoke(this, EventArgs.Empty);
        }

        public void Collapse()
        {
            if (isExpandable_)
            {
                isExpanded_ = false;

                // hide content
                this.Invalidate();
                this.Update();
                this.Size = new System.Drawing.Size(this.ClientSize.Width, rowHeight_);

                // update image
                pictureBox_.BackgroundImage = ExpandImage;
                pictureBox_.Refresh();

                // update parent layout
                var owner = PropertyTree as PropertyTree;
                if (owner != null)
                {
                    owner.UpdateLayout(this);
                }
            }
        }

        public void Clear()
        {
            Nodes.Clear();
        }


        public void UpdateSize()
        {
            // PropertyTree.SuspendLayout();
            //  WinformsUtil.SuspendDrawing(this.Parent);

            UpdateLabelSize(this.ClientSize.Width/2);

            // WinformsUtil.ResumeDrawing(this.Parent);
            // PropertyTree.ResumeLayout();
            // PropertyTree.Invalidate();
        }

        private bool NodeIsReallyVisible(Control parent, Control child)
        {
            var ptsParent = parent.PointToScreen(Point.Empty);
            var ptsChild = child.PointToScreen(Point.Empty);

            var rect1 = new Rectangle(ptsParent, parent.Size);
            var rect2 = new Rectangle(ptsChild, child.Size);

            return rect1.IntersectsWith(rect2);
        }

        private void UpdateLabelSize(int width)
        {
            if (propertyControl_ != null)
            {
                if (NodeIsReallyVisible(PropertyTree.Parent, this))
                {
                    text_.Size = new Size(width - Appearance.TextSpacing, text_.Height);

                    propertyControl_.SetBounds(
                        width, rowHeight_ / 2 - propertyControl_.Height / 2,
                        Width - width - Appearance.Indentation, propertyControl_.Size.Height);
                }
            }
            else
            {
                var children = Nodes.OfType<PropertyTreeNode>();
                foreach (var cat in children)
                {
                    cat.UpdateLabelSize(width - Appearance.Indentation);
                }
            }
        }

        public void Expand()
        {
            if (isExpandable_)
            {
                isExpanded_ = true;

                // update image
                pictureBox_.BackgroundImage = CollapseImage;
                pictureBox_.Refresh();

                // update parent layout
                var owner = PropertyTree as PropertyTree;
                if (owner != null)
                {
                    owner.UpdateLayout(this);
                }
            }
        }

        private void Nodes_NodeAdded(object sender, PropertyTreeNodeCollectionEventArgs e)
        {
            var propControl = e.Node.propertyControl_;

            if (propControl == null )
            {
                e.Node.Parent = this;
            }
            else
            {
                int rowHeight = Appearance.ControlRowHeight;
                int lblWidth = (int)(220.0f * WinformsUtil.Scaling);

                int offset = 0;
                var p = this.Parent;
                while (p != null)
                {
                    offset += Appearance.Indentation;
                    p = p.Parent;
                }
                lblWidth -= offset;
                lblWidth -= Appearance.TextSpacing;

                var c = e.Node;

                c.Parent = this;
                c.rowHeight_ = rowHeight;
                c.isExpanded_ = false;
                c.isExpandable_ = false;
                c.BackColor = SystemColors.ControlLightLight;
                c.Size = new Size(c.Width, rowHeight);

                // hide expand icon
                c.pictureBox_.Visible = false;

                // update label text
                c.text_.Location = new System.Drawing.Point(0, 0);
                c.text_.Size = new Size(lblWidth - Appearance.TextSpacing, rowHeight);
                c.text_.Anchor = AnchorStyles.Top | AnchorStyles.Left;
                c.text_.BackColor = SystemColors.ControlLightLight;
                c.text_.TextAlign = ContentAlignment.MiddleRight;

                // property control
                c.propertyControl_ = propControl;
                c.propertyControl_.Parent = c;
                c.propertyControl_.Size = new Size(c.Width - lblWidth - Appearance.Indentation, c.propertyControl_.Size.Height);
                c.propertyControl_.Location = new Point(lblWidth, rowHeight / 2 - c.propertyControl_.Height / 2);
                c.propertyControl_.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
                c.propertyControl_.BackColor = SystemColors.ControlLightLight;
                c.propertyControl_.MaximumSize = new Size(Appearance.MaximumControlWidth, c.propertyControl_.Size.Height);

                c.isExpandable_ = false;
                
            }
        }

        private void Nodes__NodeRemoved(object sender, PropertyTreeNodeCollectionEventArgs e)
        {
            var nodeToRemove = e.Node;
            this.Controls.Remove((Control)nodeToRemove);
            var children = nodeToRemove.Nodes.OfType<PropertyTreeNode>().ToList();
            foreach (var node in children)
            {
                nodeToRemove.Nodes.Remove(node);
            }
        }

    }
}
