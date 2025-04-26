<?xml version="1.0" encoding="UTF-8"?>
<ui version="4.0">
 <class>DgVoodoo2Dialog</class>
 <widget class="QDialog" name="DgVoodoo2Dialog">
  <property name="geometry">
   <rect>
    <x>0</x>
    <y>0</y>
    <width>450</width>
    <height>450</height>
   </rect>
  </property>
  <property name="windowTitle">
   <string>DgVoodoo2 D3D Wrapper Settings</string>
  </property>
  <layout class="QVBoxLayout" name="verticalLayout">
   <item>
    <widget class="QGroupBox" name="groupBoxPath">
     <property name="title">
      <string>DgVoodoo2 Path</string>
     </property>
     <layout class="QHBoxLayout" name="horizontalLayout">
      <item>
       <widget class="QLineEdit" name="lineEditPath"/>
      </item>
      <item>
       <widget class="QPushButton" name="pushButtonBrowse">
        <property name="text">
         <string>Browse...</string>
        </property>
       </widget>
      </item>
     </layout>
    </widget>
   </item>
   <item>
    <widget class="QGroupBox" name="groupBoxGPU">
     <property name="title">
      <string>GPU Emulation</string>
     </property>
     <layout class="QGridLayout" name="gridLayout">
      <item row="0" column="0">
       <widget class="QLabel" name="labelVendor">
        <property name="text">
         <string>GPU Vendor:</string>
        </property>
       </widget>
      </item>
      <item row="0" column="1">
       <widget class="QComboBox" name="comboBoxGPUVendor"/>
      </item>
      <item row="1" column="0">
       <widget class="QLabel" name="labelModel">
        <property name="text">
         <string>GPU Model:</string>
        </property>
       </widget>
      </item>
      <item row="1" column="1">
       <widget class="QComboBox" name="comboBoxGPUModel"/>
      </item>
     </layout>
    </widget>
   </item>
   <item>
    <widget class="QGroupBox" name="groupBoxBusType">
     <property name="title">
      <string>Bus Type</string>
     </property>
     <layout class="QHBoxLayout" name="horizontalLayout_2">
      <item>
       <widget class="QRadioButton" name="radioButtonPCI">
        <property name="text">
         <string>PCI</string>
        </property>
       </widget>
      </item>
      <item>
       <widget class="QRadioButton" name="radioButtonAGP">
        <property name="text">
         <string>AGP (if supported)</string>
        </property>
        <property name="checked">
         <bool>true</bool>
        </property>
       </widget>
      </item>
     </layout>
    </widget>
   </item>
   <item>
    <widget class="QGroupBox" name="groupBoxWrappers">
     <property name="title">
      <string>Wrapper DLLs</string>
     </property>
     <layout class="QVBoxLayout" name="verticalLayout_2">
      <item>
       <widget class="QCheckBox" name="checkBoxDDraw">
        <property name="text">
         <string>DirectDraw (DDraw.dll)</string>
        </property>
       </widget>
      </item>
      <item>
       <widget class="QCheckBox" name="checkBoxD3DImm">
        <property name="text">
         <string>Direct3D Immediate Mode (D3DImm.dll)</string>
        </property>
       </widget>
      </item>
      <item>
       <widget class="QCheckBox" name="checkBoxD3D8">
        <property name="text">
         <string>Direct3D 8 (D3D8.dll)</string>
        </property>
       </widget>
      </item>
      <item>
       <widget class="QCheckBox" name="checkBoxD3D9">
        <property name="text">
         <string>Direct3D 9 (D3D9.dll)</string>
        </property>
       </widget>
      </item>
     </layout>
    </widget>
   </item>
   <item>
    <widget class="QLabel" name="labelWarning">
     <property name="text">
      <string></string>
     </property>
     <property name="wordWrap">
      <bool>true</bool>
     </property>
     <property name="textInteractionFlags">
      <set>Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse</set>
     </property>
    </widget>
   </item>
   <item>
    <widget class="QLabel" name="labelNote">
     <property name="text">
      <string>Note: When enabled, DgVoodoo2 will create a virtual PCI/AGP 3D accelerator that wraps DirectDraw/Direct3D calls to modern graphics APIs.</string>
     </property>
     <property name="wordWrap">
      <bool>true</bool>
     </property>
    </widget>
   </item>
   <item>
    <widget class="QDialogButtonBox" name="buttonBox">
     <property name="orientation">
      <enum>Qt::Horizontal</enum>
     </property>
     <property name="standardButtons">
      <set>QDialogButtonBox::Cancel|QDialogButtonBox::Ok</set>
     </property>
    </widget>
   </item>
  </layout>
 </widget>
 <resources/>
 <connections>
  <connection>
   <sender>buttonBox</sender>
   <signal>accepted()</signal>
   <receiver>DgVoodoo2Dialog</receiver>
   <slot>on_pushButtonOK_clicked()</slot>
   <hints>
    <hint type="sourcelabel">
     <x>248</x>
     <y>254</y>
    </hint>
    <hint type="destinationlabel">
     <x>157</x>
     <y>274</y>
    </hint>
   </hints>
  </connection>
  <connection>
   <sender>buttonBox</sender>
   <signal>rejected()</signal>
   <receiver>DgVoodoo2Dialog</receiver>
   <slot>on_pushButtonCancel_clicked()</slot>
   <hints>
    <hint type="sourcelabel">
     <x>316</x>
     <y>260</y>
    </hint>
    <hint type="destinationlabel">
     <x>286</x>
     <y>274</y>
    </hint>
   </hints>
  </connection>
 </connections>
</ui>