using ic4;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows.Forms;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;

namespace ic4.Examples
{
    public enum PropertyViewFlags
    {
        Default = 0,
        AllowStreamRestart = 1
    }

    [ToolboxItem(false)]
    public class CustomPanel : System.Windows.Forms.Panel
    {
        protected override System.Drawing.Point ScrollToControl(System.Windows.Forms.Control activeControl)
        {
            // Returning the current location prevents the panel from
            // scrolling to the active control when the panel loses and regains focus
            return this.DisplayRectangle.Location;
        }
    }

    [ToolboxItem(true)]
    public partial class PropertyView : UserControl
    {
        private ic4.Grabber grabber_;
        private PropertyTree propertyTree_;
        private System.Windows.Forms.SplitContainer splitContainer_;

        private Dictionary<string, PropertyTreeNode> propertyTreeNodeCache_ = new Dictionary<string, PropertyTreeNode>();

        private ic4.PropertyVisibility visiblility_ = ic4.PropertyVisibility.Beginner;
        private string filter_ = string.Empty;

        public PropertyView(PropertyViewFlags flags = PropertyViewFlags.Default)
        {
            if (!DesignMode)
            {
                WinformsUtil.Init(this);
            }
            PropControlBase.Flags = flags;
            InitializeComponent();
        }
        
        public PropertyView(ic4.Grabber grabber, PropertyViewFlags flags = PropertyViewFlags.Default)
        {
            if (!DesignMode)
            {
                WinformsUtil.Init(this);
            }
            PropControlBase.Flags = flags;
            InitializeComponent();
            Reset(grabber);
        }

        public void SuspendDrawing()
        {
            WinformsUtil.SuspendDrawing(splitContainer_);
            WinformsUtil.SuspendDrawing(propertyTree_);
        }

        public void ResumeDrawing()
        {
            WinformsUtil.ResumeDrawing(propertyTree_);
            WinformsUtil.ResumeDrawing(splitContainer_);
        }

        public void Reset(ic4.Grabber grabber)
        {
            if(grabber == null)
            {
                throw new ArgumentNullException("Grabber is null!");
            }

            if(grabber != grabber_)
            {
                grabber_ = grabber;
                propertyTreeNodeCache_.Clear();
            }

            SuspendLayout();
            SuspendDrawing();

            ClearPropertyTree();
            RecreatePropertyTree();

            ResumeLayout();
            ResumeDrawing();
        }

        private void ClearPropertyTree()
        {
            propertyTree_.Clear();
            propertyTree_.Refresh();
        }

        private void RecreatePropertyTree()
        {
            var root = grabber_.DevicePropertyMap.FindCategory("Root");
            InitProperties(propertyTree_.Root, root.Features);
            propertyTree_.UpdateLayout(null);
        }

        private bool TryGetPropertyTreeNode(Property property, out PropertyTreeNode subNode)
        {
            if (propertyTreeNodeCache_.TryGetValue(property.Name, out subNode))
            {
                var ctrl = subNode.PropertyControl as PropControlBase;
                if (ctrl.Property.Type != property.Type)
                {
                    propertyTreeNodeCache_.Remove(property.Name);
                    subNode = null;
                    return false;
                }
                else
                {
                    ctrl.Property = property;
                    ctrl.UpdateAll();
                }
            }
            return false;
        }

        private void InitProperties(PropertyTreeNode node, IEnumerable<Property> properties)
        {
            foreach( var property in properties)
            {
                
                if (property.Type == PropertyType.Category)
                {
                    var category = property as PropCategory;
                    var subNode = new PropertyTreeNode(category.DisplayName, null);

                    InitProperties(subNode, category.Features);

                    if (subNode.Nodes.Count > 0)
                    {
                        node.Nodes.Add(subNode);
                    }
                }
                else
                {
                    if (!ShouldShow(property, filter_, visiblility_))
                        continue;

                    try
                    {
                        PropertyTreeNode subNode = null;
                        if (!TryGetPropertyTreeNode(property, out subNode))
                        {
                            PropControlBase ctrl = null;

                            switch (property.Type)
                            {
                                case PropertyType.Integer:
                                    ctrl = new PropIntegerControl(grabber_, property as ic4.PropInteger);
                                    break;
                                case PropertyType.Float:
                                    ctrl = new PropFloatControl(grabber_, property as ic4.PropFloat);
                                    break;
                                case PropertyType.Boolean:
                                    ctrl = new PropBooleanControl(grabber_, property as ic4.PropBoolean);
                                    break;
                                case PropertyType.String:
                                    ctrl = new PropStringControl(grabber_, property as ic4.PropString);
                                    break;
                                case PropertyType.Enumeration:
                                    ctrl = new PropEnumerationControl(grabber_, property as ic4.PropEnumeration);
                                    break;
                                case PropertyType.Command:
                                    ctrl = new PropCommandControl(grabber_, property as ic4.PropCommand);
                                    break;
                                default:
                                    break;
                            }

                            if (ctrl != null)
                            {
                                subNode = new PropertyTreeNode(property.DisplayName, ctrl);
                                subNode.NodeSlected += (s, e) =>
                                {
                                    var n = s as PropertyTreeNode;
                                    UpdateInfoBox((n.PropertyControl as PropControlBase).Property);
                                };
                                propertyTreeNodeCache_[property.Name] = subNode;
                            }
                        }

                        if (subNode != null)
                        {
                            node.Nodes.Add(subNode);
                        }
                    }
                    catch (Exception ex)
                    {
                        System.Console.WriteLine(ex.Message);
                    }
                }
            }
        }

        private static bool ShouldShow(Property property, string filter, ic4.PropertyVisibility visibility)
        {
            var regex = new Regex("([(,|\\|)])");
            var filters = regex.Split(filter);
            var propDisplayName = property.DisplayName.ToLower();
            var propName = property.Name.ToLower();
            var propVis = property.Visibility;

            if (propVis > visibility)
                return false;

            if (filters.Count() == 0)
                return true;

            foreach (var f in filters)
            {
                var text = f.ToLower();

                if (propDisplayName.Contains(text))
                    return true;

                if (propName.Contains(text))
                    return true;
            }

            return false;
        }

        private string CreateTableRow(string desc, string val)
        {
            float width = (float)((((double)txtDescription_.Width) * 1440.0) / ((double)WinformsUtil.DPI));
            var cellx0 = (int)(width * 0.2f);
            var cellx1 = cellx0 + (int)(width * 0.5f);
            return string.Format(@"\trowd \cellx{0} \cellx{1} \intbl {2} \cell \intbl {3} \cell \row ", cellx0, cellx1, desc, val);
        }

        private void UpdateInfoBox(Property prop)
        {
            var name = prop.DisplayName;
            var desc = prop.Description;

            string text = @"{\rtf1 " + string.Format(@"\b {0} \b0 \line \line {1}", name, desc);
            text += @" \line ";

            bool isLocked = prop.IsLocked;
            bool isReadonly = prop.IsReadonly;

            text += CreateTableRow("", "");

            if (isReadonly)
            {
                text += CreateTableRow(" Access", " Read-Only");
            }
            else if (isLocked)
            {
                text += CreateTableRow(" Access", " Readable, Locked");
            }
            else
            {
                text += CreateTableRow(" Access", " Readable, Writable");
            }


            switch (prop.Type)
            {
                case PropertyType.Category:
                    text += CreateTableRow(" Type", " Category");
                    break;
                case PropertyType.Integer:
                    text += CreateTableRow(" Type", " Integer");
                    {
                        var intProp = prop as PropInteger;
                        var rep = intProp.Representation;

                        try
                        {
                            text += CreateTableRow(" Value", " " + intProp.Value.ToString());
                        }
                        catch(Exception ex)
                        {
                            text += CreateTableRow(" Value", " " + ex.Message);
                        }

                        if(!intProp.IsReadonly)
                        {
                            text += CreateTableRow(" Minimum", " " + intProp.Minimum);
                            text += CreateTableRow(" Maximum", " " + intProp.Maximum);

                            if(intProp.IncrementMode != PropertyIncrementMode.None)
                                text += CreateTableRow(" Increment", " " + intProp.Increment);
                        }
                    }
                    break;
                case PropertyType.String:
                    text += CreateTableRow(" Type", " String");
                    {
                        var stringProp = prop as PropString;
                        try
                        {
                            text += CreateTableRow(" Value", " " + stringProp.Value);
                        }
                        catch (Exception ex)
                        {
                            text += CreateTableRow(" Value", " " + ex.Message);
                        }

                        if(!stringProp.IsReadonly)
                        {
                            text += CreateTableRow(" Maximum Length", " " + stringProp.MaxLength.ToString());
                        }
                    }
                    break;
                case PropertyType.Float:
                    var floatProp = prop as PropFloat;
                    text += CreateTableRow(" Type", " Integer");
                    text += CreateTableRow(" Unit", " " + floatProp.Unit);
                    {
                      
      
                        try
                        {
                            text += CreateTableRow(" Value", " " + floatProp.Value.ToString());
                        }
                        catch (Exception ex)
                        {
                            text += CreateTableRow(" Value", " " + ex.Message);
                        }

                        if (!floatProp.IsReadonly)
                        {
                            text += CreateTableRow(" Minimum", " " + floatProp.Minimum);
                            text += CreateTableRow(" Maximum", " " + floatProp.Maximum);

                            if (floatProp.IncrementMode != PropertyIncrementMode.None)
                                text += CreateTableRow(" Increment", " " + floatProp.Increment);
                        }
                    }
                    break;
                default:
                    break;
            }

            text += "}";

            txtDescription_.Rtf = text;
            txtDescription_.ReadOnly = true;
        }

        private void CboVisibility_SelectedIndexChanged(object sender, System.EventArgs e)
        {
            if (Enum.TryParse<ic4.PropertyVisibility>(cboVisibility.SelectedItem.ToString(), out var visiblility))
            {
                if (visiblility_ != visiblility)
                {
                    visiblility_ = visiblility;

                    ClearPropertyTree();

                    this.SuspendLayout();
                    WinformsUtil.SuspendDrawing(splitContainer_);
                    WinformsUtil.SuspendDrawing(propertyTree_);

                    RecreatePropertyTree();

                    this.ResumeLayout();
                    WinformsUtil.ResumeDrawing(propertyTree_);
                    WinformsUtil.ResumeDrawing(splitContainer_);
                }
            }
        }

        private void TxtFilter_TextChanged(object sender, System.EventArgs e)
        {
            if (filter_ != txtFilter.Text)
            {
                filter_ = txtFilter.Text;

                this.SuspendLayout();
                WinformsUtil.SuspendDrawing(splitContainer_);
                WinformsUtil.SuspendDrawing(propertyTree_);

                ClearPropertyTree();
                RecreatePropertyTree();
                propertyTree_.Root.UpdateSize();

                this.ResumeLayout();
                WinformsUtil.ResumeDrawing(propertyTree_);
                WinformsUtil.ResumeDrawing(splitContainer_);
            }
        }

       
    }

    [ToolboxItem(false)]
    class PropertyTree : Panel
    {
        private PropertyTreeNode root_;

        public PropertyTreeNode Root => root_;

        protected bool IsDesignMode
        {
            get { return DesignMode || LicenseManager.UsageMode == LicenseUsageMode.Designtime; }
        }

        public PropertyTree()
        {
            if (!IsDesignMode)
            {
                this.root_ = new PropertyTreeNode("root", null);
                this.root_.Parent = this;
                this.root_.ShowHeader = false;
                this.DoubleBuffered = true;
            }
            this.DoubleBuffered = true;
            this.BackColor = SystemColors.ControlLight;
            this.SizeChanged += PropertyTree_SizeChanged;
            SetStyle(System.Windows.Forms.ControlStyles.OptimizedDoubleBuffer, true);
        }

        protected override void OnParentChanged(EventArgs e)
        {
            base.OnParentChanged(e);

            if(Parent != null)
            {
                var ctrl = Parent as Panel;
                ctrl.Scroll += (s, ee) =>
                {
                    UpdateNodesLocationAndSize();
                };
                ctrl.MouseWheel += (s, ee) =>
                {
                    UpdateNodesLocationAndSize();
                };
            }
        }

        public void SuspendDrawing()
        {
            WinformsUtil.SuspendDrawing(root_);
        }

        public void ResumeDrawing()
        {
            WinformsUtil.ResumeDrawing(root_);
        }

        private void UpdateNodesLocationAndSize()
        {
            if (!IsDesignMode)
            {
                if (root_ != null)
                {
                    //var ctrl = this.Parent as CustomPanel;
                    //System.Console.WriteLine(ctrl.AutoScrollPosition.Y);
                    //var sw = new Stopwatch();
                    //sw.Start();

                    root_.UpdateSize();

                    //sw.Stop();
                    //System.Console.WriteLine("Time: " + sw.ElapsedMilliseconds);
                    //System.Console.WriteLine("AutoScrollPosition: " + ctrl.AutoScrollPosition.Y);
                }
            }
        }

        private void PropertyTree_SizeChanged(object sender, EventArgs e)
        {
            UpdateNodesLocationAndSize();
        }

        public void UpdateLayout(PropertyTreeNode caller)
        {
            if (!IsDesignMode)
            {
                if (root_ != null)
                {
                    root_.Updatelayout(0, 0);
                    this.Size = root_.Size;
                }
            }
        }

        public void Clear()
        {
            if (root_ != null)
            {
                root_.Clear();
                root_.Updatelayout(0, 0);
            }
        }
    }
}
