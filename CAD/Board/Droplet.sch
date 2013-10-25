<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="6.1">
<drawing>
<settings>
<setting alwaysvectorfont="yes"/>
<setting verticaltext="up"/>
</settings>
<grid distance="0.1" unitdist="inch" unit="inch" style="lines" multiple="1" display="no" altdistance="0.01" altunitdist="inch" altunit="inch"/>
<layers>
<layer number="1" name="Top" color="4" fill="1" visible="no" active="no"/>
<layer number="16" name="Bottom" color="1" fill="1" visible="no" active="no"/>
<layer number="17" name="Pads" color="2" fill="1" visible="no" active="no"/>
<layer number="18" name="Vias" color="2" fill="1" visible="no" active="no"/>
<layer number="19" name="Unrouted" color="6" fill="1" visible="no" active="no"/>
<layer number="20" name="Dimension" color="15" fill="1" visible="no" active="no"/>
<layer number="21" name="tPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="22" name="bPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="23" name="tOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="24" name="bOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="25" name="tNames" color="7" fill="1" visible="no" active="no"/>
<layer number="26" name="bNames" color="7" fill="1" visible="no" active="no"/>
<layer number="27" name="tValues" color="7" fill="1" visible="no" active="no"/>
<layer number="28" name="bValues" color="7" fill="1" visible="no" active="no"/>
<layer number="29" name="tStop" color="7" fill="3" visible="no" active="no"/>
<layer number="30" name="bStop" color="7" fill="6" visible="no" active="no"/>
<layer number="31" name="tCream" color="7" fill="4" visible="no" active="no"/>
<layer number="32" name="bCream" color="7" fill="5" visible="no" active="no"/>
<layer number="33" name="tFinish" color="6" fill="3" visible="no" active="no"/>
<layer number="34" name="bFinish" color="6" fill="6" visible="no" active="no"/>
<layer number="35" name="tGlue" color="7" fill="4" visible="no" active="no"/>
<layer number="36" name="bGlue" color="7" fill="5" visible="no" active="no"/>
<layer number="37" name="tTest" color="7" fill="1" visible="no" active="no"/>
<layer number="38" name="bTest" color="7" fill="1" visible="no" active="no"/>
<layer number="39" name="tKeepout" color="4" fill="11" visible="no" active="no"/>
<layer number="40" name="bKeepout" color="1" fill="11" visible="no" active="no"/>
<layer number="41" name="tRestrict" color="4" fill="10" visible="no" active="no"/>
<layer number="42" name="bRestrict" color="1" fill="10" visible="no" active="no"/>
<layer number="43" name="vRestrict" color="2" fill="10" visible="no" active="no"/>
<layer number="44" name="Drills" color="7" fill="1" visible="no" active="no"/>
<layer number="45" name="Holes" color="7" fill="1" visible="no" active="no"/>
<layer number="46" name="Milling" color="3" fill="1" visible="no" active="no"/>
<layer number="47" name="Measures" color="7" fill="1" visible="no" active="no"/>
<layer number="48" name="Document" color="7" fill="1" visible="no" active="no"/>
<layer number="49" name="Reference" color="7" fill="1" visible="no" active="no"/>
<layer number="50" name="dxf" color="7" fill="1" visible="no" active="no"/>
<layer number="51" name="tDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="52" name="bDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="91" name="Nets" color="2" fill="1" visible="yes" active="yes"/>
<layer number="92" name="Busses" color="1" fill="1" visible="yes" active="yes"/>
<layer number="93" name="Pins" color="2" fill="1" visible="no" active="yes"/>
<layer number="94" name="Symbols" color="4" fill="1" visible="yes" active="yes"/>
<layer number="95" name="Names" color="7" fill="1" visible="yes" active="yes"/>
<layer number="96" name="Values" color="7" fill="1" visible="yes" active="yes"/>
<layer number="97" name="Info" color="7" fill="1" visible="yes" active="yes"/>
<layer number="98" name="Guide" color="6" fill="1" visible="yes" active="yes"/>
<layer number="200" name="200bmp" color="7" fill="1" visible="yes" active="yes"/>
<layer number="250" name="Descript" color="7" fill="1" visible="yes" active="yes"/>
<layer number="251" name="SMDround" color="7" fill="1" visible="yes" active="yes"/>
</layers>
<schematic xreflabel="%F%N/%S.%C%R" xrefpart="/%S.%C%R">
<libraries>
<library name="Droplet">
<packages>
<package name="0402">
<wire x1="-0.245" y1="0.224" x2="0.245" y2="0.224" width="0.1524" layer="51"/>
<wire x1="0.245" y1="-0.224" x2="-0.245" y2="-0.224" width="0.1524" layer="51"/>
<smd name="1" x="-0.65" y="0" dx="0.7" dy="0.9" layer="1"/>
<smd name="2" x="0.65" y="0" dx="0.7" dy="0.9" layer="1"/>
<rectangle x1="-0.554" y1="-0.3048" x2="-0.254" y2="0.2951" layer="51"/>
<rectangle x1="0.2588" y1="-0.3048" x2="0.5588" y2="0.2951" layer="51"/>
<text x="1.0668" y="-0.2794" size="0.635" layer="21" ratio="15">&gt;NAME</text>
<wire x1="-0.9144" y1="0.5334" x2="0.9144" y2="0.5334" width="0.127" layer="21"/>
<wire x1="-0.9144" y1="-0.5334" x2="0.9144" y2="-0.5334" width="0.127" layer="21"/>
</package>
<package name="0603">
<wire x1="-0.432" y1="-0.356" x2="0.432" y2="-0.356" width="0.1524" layer="51"/>
<wire x1="0.432" y1="0.356" x2="-0.432" y2="0.356" width="0.1524" layer="51"/>
<smd name="1" x="-0.85" y="0" dx="1" dy="1.1" layer="1"/>
<smd name="2" x="0.85" y="0" dx="1" dy="1.1" layer="1"/>
<rectangle x1="0.4318" y1="-0.4318" x2="0.8382" y2="0.4318" layer="51"/>
<rectangle x1="-0.8382" y1="-0.4318" x2="-0.4318" y2="0.4318" layer="51"/>
<text x="1.4732" y="-0.3302" size="0.635" layer="21" ratio="15">&gt;NAME</text>
<wire x1="-1.2446" y1="0.635" x2="1.2446" y2="0.635" width="0.127" layer="21"/>
<wire x1="-1.2446" y1="-0.635" x2="1.2446" y2="-0.635" width="0.127" layer="21"/>
</package>
<package name="LEG">
<pad name="1" x="0" y="0" drill="0.8" diameter="1.8"/>
<circle x="0" y="0" radius="1.905" width="0.127" layer="39"/>
<circle x="0" y="0" radius="0.9" width="1" layer="43"/>
</package>
<package name="SUPERCAP">
<wire x1="10.9914" y1="2.7288" x2="10.9914" y2="1.8048" width="0.127" layer="21"/>
<wire x1="10.5294" y1="2.2668" x2="11.4534" y2="2.2668" width="0.127" layer="21"/>
<circle x="0" y="0" radius="9.5" width="0.127" layer="51"/>
<pad name="N" x="-10" y="0" drill="1.25" shape="long" rot="R90"/>
<pad name="P" x="10" y="0" drill="1.25" shape="long" rot="R90"/>
<rectangle x1="-10.25" y1="-3.5" x2="-9.75" y2="3.5" layer="39"/>
<rectangle x1="9.75" y1="-3.5" x2="10.25" y2="3.5" layer="39"/>
<wire x1="10.9914" y1="2.7288" x2="10.9914" y2="1.8048" width="0.127" layer="22"/>
<wire x1="10.5294" y1="2.2668" x2="11.4534" y2="2.2668" width="0.127" layer="22"/>
</package>
<package name="SOT-25">
<wire x1="-0.8" y1="1.45" x2="0.8" y2="1.45" width="0.127" layer="21"/>
<wire x1="0.8" y1="1.45" x2="0.8" y2="-1.45" width="0.127" layer="51"/>
<wire x1="0.8" y1="-1.45" x2="-0.8" y2="-1.45" width="0.127" layer="21"/>
<wire x1="-0.8" y1="-1.45" x2="-0.8" y2="1.45" width="0.127" layer="51"/>
<smd name="VIN" x="-1.22" y="0.97" dx="0.8" dy="0.55" layer="1"/>
<smd name="VSS" x="-1.22" y="0" dx="0.8" dy="0.55" layer="1"/>
<smd name="CE" x="-1.22" y="-0.97" dx="0.8" dy="0.55" layer="1"/>
<smd name="VOUT" x="1.22" y="0.97" dx="0.8" dy="0.55" layer="1"/>
<smd name="NC" x="1.22" y="-0.97" dx="0.8" dy="0.55" layer="1"/>
<wire x1="0.8" y1="0.5" x2="0.8" y2="-0.5" width="0.127" layer="21"/>
<text x="-1.27" y="1.524" size="0.635" layer="21" ratio="15">&gt;NAME</text>
</package>
<package name="SOT-23-6">
<wire x1="0" y1="-1.29" x2="0" y2="-1.3" width="0.01" layer="21"/>
<wire x1="1.42" y1="0.8" x2="1.42" y2="-0.8" width="0.127" layer="21"/>
<wire x1="1.42" y1="-0.8" x2="-1.42" y2="-0.8" width="0.127" layer="51"/>
<wire x1="-1.42" y1="-0.8" x2="-1.42" y2="0.8" width="0.127" layer="21"/>
<wire x1="-1.42" y1="0.8" x2="1.42" y2="0.8" width="0.127" layer="51"/>
<smd name="1" x="-0.95" y="-1.29" dx="0.69" dy="0.99" layer="1"/>
<smd name="2" x="0" y="-1.29" dx="0.69" dy="0.99" layer="1"/>
<smd name="3" x="0.95" y="-1.29" dx="0.69" dy="0.99" layer="1"/>
<smd name="4" x="0.95" y="1.3" dx="0.69" dy="0.99" layer="1"/>
<smd name="6" x="-0.95" y="1.3" dx="0.69" dy="0.99" layer="1"/>
<smd name="5" x="0" y="1.3" dx="0.69" dy="0.99" layer="1"/>
<rectangle x1="-1.11" y1="0.8" x2="-0.78" y2="1.43" layer="51"/>
<rectangle x1="0.79" y1="0.8" x2="1.12" y2="1.42" layer="51"/>
<rectangle x1="-1.11" y1="-1.42" x2="-0.78" y2="-0.8" layer="51"/>
<rectangle x1="-0.16" y1="-1.42" x2="0.17" y2="-0.8" layer="51"/>
<rectangle x1="0.79" y1="-1.42" x2="1.12" y2="-0.8" layer="51"/>
<rectangle x1="-0.16" y1="0.8" x2="0.17" y2="1.42" layer="51"/>
<rectangle x1="-1.375" y1="-0.75" x2="-0.625" y2="0" layer="21"/>
<text x="-1.524" y="-1.016" size="0.635" layer="21" ratio="15" rot="R90">&gt;NAME</text>
</package>
<package name="1208-IR">
<wire x1="1" y1="-0.9" x2="0.3" y2="-1.7" width="0.127" layer="21" curve="-97.611926"/>
<wire x1="-0.3" y1="-1.7" x2="-1" y2="-0.9" width="0.127" layer="21" curve="-97.62815"/>
<wire x1="-0.3" y1="-1.7" x2="0.3" y2="-1.7" width="0.127" layer="21"/>
<wire x1="0.7254" y1="0.546" x2="0.7254" y2="-0.546" width="0.3048" layer="21"/>
<smd name="A" x="-1.75" y="0" dx="1.5" dy="1.5" layer="1"/>
<smd name="C" x="1.75" y="0" dx="1.5" dy="1.5" layer="1"/>
<text x="2.6162" y="-0.2794" size="0.635" layer="21" ratio="15">&gt;NAME</text>
</package>
<package name="IR_RECEIVER">
<pad name="1" x="-2.54" y="0" drill="0.8"/>
<pad name="2" x="0" y="0" drill="0.8"/>
<pad name="3" x="2.54" y="0" drill="0.8"/>
<rectangle x1="-4.25" y1="-4.5" x2="4.25" y2="2.75" layer="39"/>
<wire x1="-2.4892" y1="0.7874" x2="-2.4892" y2="1.1938" width="0.127" layer="21"/>
<wire x1="-2.4892" y1="1.1938" x2="-1.905" y2="1.1938" width="0.127" layer="21"/>
<wire x1="2.4892" y1="0.7874" x2="2.4892" y2="1.1938" width="0.127" layer="21"/>
<wire x1="2.4892" y1="1.1938" x2="1.8796" y2="1.1938" width="0.127" layer="21"/>
<wire x1="2" y1="-1.5" x2="-2" y2="-1.6" width="0.127" layer="51" curve="-180"/>
<wire x1="-2.5" y1="-0.8" x2="-2.5" y2="-1.6" width="0.127" layer="51"/>
<wire x1="-2.5" y1="-1.6" x2="-2" y2="-1.6" width="0.127" layer="51"/>
<wire x1="2" y1="-1.5" x2="2.5" y2="-1.5" width="0.127" layer="51"/>
<wire x1="2.5" y1="-1.5" x2="2.5" y2="-0.8" width="0.127" layer="51"/>
</package>
<package name="QFN-20">
<wire x1="-2" y1="1.3" x2="-2" y2="2" width="0.127" layer="21"/>
<wire x1="-2" y1="2" x2="-1.3" y2="2" width="0.127" layer="21"/>
<wire x1="1.3" y1="2" x2="2" y2="2" width="0.127" layer="21"/>
<wire x1="2" y1="2" x2="2" y2="1.3" width="0.127" layer="21"/>
<wire x1="2" y1="-1.3" x2="2" y2="-2" width="0.127" layer="21"/>
<wire x1="2" y1="-2" x2="1.3" y2="-2" width="0.127" layer="21"/>
<wire x1="-1.3" y1="-2" x2="-2" y2="-2" width="0.127" layer="21"/>
<wire x1="-2" y1="-2" x2="-2" y2="-1.3" width="0.127" layer="21"/>
<circle x="-1.238" y="1.23" radius="0.1414" width="0.254" layer="21"/>
<smd name="8" x="0" y="-2.1" dx="0.3" dy="0.9" layer="1"/>
<smd name="9" x="0.5" y="-2.1" dx="0.3" dy="0.9" layer="1"/>
<smd name="10" x="1" y="-2.1" dx="0.3" dy="0.9" layer="1"/>
<smd name="7" x="-0.5" y="-2.1" dx="0.3" dy="0.9" layer="1"/>
<smd name="6" x="-1" y="-2.1" dx="0.3" dy="0.9" layer="1"/>
<smd name="5" x="-2.1" y="-1" dx="0.3" dy="0.9" layer="1" rot="R90"/>
<smd name="4" x="-2.1" y="-0.5" dx="0.3" dy="0.9" layer="1" rot="R90"/>
<smd name="3" x="-2.1" y="0" dx="0.3" dy="0.9" layer="1" rot="R90"/>
<smd name="2" x="-2.1" y="0.5" dx="0.3" dy="0.9" layer="1" rot="R90"/>
<smd name="1" x="-2.1" y="1" dx="0.3" dy="0.9" layer="1" rot="R90"/>
<smd name="11" x="2.1" y="-1" dx="0.3" dy="0.9" layer="1" rot="R90"/>
<smd name="12" x="2.1" y="-0.5" dx="0.3" dy="0.9" layer="1" rot="R90"/>
<smd name="13" x="2.1" y="0" dx="0.3" dy="0.9" layer="1" rot="R90"/>
<smd name="14" x="2.1" y="0.5" dx="0.3" dy="0.9" layer="1" rot="R90"/>
<smd name="15" x="2.1" y="1" dx="0.3" dy="0.9" layer="1" rot="R90"/>
<smd name="16" x="1" y="2.1" dx="0.3" dy="0.9" layer="1" rot="R180"/>
<smd name="17" x="0.5" y="2.1" dx="0.3" dy="0.9" layer="1" rot="R180"/>
<smd name="18" x="0" y="2.1" dx="0.3" dy="0.9" layer="1" rot="R180"/>
<smd name="19" x="-0.5" y="2.1" dx="0.3" dy="0.9" layer="1" rot="R180"/>
<smd name="20" x="-1" y="2.1" dx="0.3" dy="0.9" layer="1" rot="R180"/>
<text x="-2.032" y="2.54" size="0.635" layer="21" ratio="15">&gt;NAME</text>
</package>
<package name="DHVQFN14-L">
<circle x="-0.684" y="-0.338" radius="0.1414" width="0.254" layer="21"/>
<smd name="14" x="-1.525" y="0.25" dx="0.3" dy="0.9" layer="1" rot="R270"/>
<smd name="13" x="-1" y="1.275" dx="0.3" dy="0.9" layer="1" rot="R180"/>
<smd name="12" x="-0.5" y="1.275" dx="0.3" dy="0.9" layer="1" rot="R180"/>
<smd name="11" x="0" y="1.275" dx="0.3" dy="0.9" layer="1" rot="R180"/>
<smd name="10" x="0.5" y="1.275" dx="0.3" dy="0.9" layer="1" rot="R180"/>
<smd name="9" x="1" y="1.275" dx="0.3" dy="0.9" layer="1" rot="R180"/>
<smd name="6" x="1" y="-1.275" dx="0.3" dy="0.9" layer="1" rot="R180"/>
<smd name="5" x="0.5" y="-1.275" dx="0.3" dy="0.9" layer="1" rot="R180"/>
<smd name="4" x="0" y="-1.275" dx="0.3" dy="0.9" layer="1" rot="R180"/>
<smd name="3" x="-0.5" y="-1.275" dx="0.3" dy="0.9" layer="1" rot="R180"/>
<smd name="2" x="-1" y="-1.275" dx="0.3" dy="0.9" layer="1" rot="R180"/>
<smd name="1" x="-1.525" y="-0.25" dx="0.3" dy="0.9" layer="1" rot="R270"/>
<smd name="8" x="1.525" y="0.25" dx="0.3" dy="0.9" layer="1" rot="R270"/>
<smd name="7" x="1.525" y="-0.25" dx="0.3" dy="0.9" layer="1" rot="R270"/>
<wire x1="-1.5" y1="0.55" x2="-1.5" y2="1.25" width="0.127" layer="21"/>
<wire x1="-1.5" y1="1.25" x2="-1.3" y2="1.25" width="0.127" layer="21"/>
<wire x1="-1.5" y1="-0.55" x2="-1.5" y2="-1.25" width="0.127" layer="21"/>
<wire x1="-1.5" y1="-1.25" x2="-1.3" y2="-1.25" width="0.127" layer="21"/>
<wire x1="1.3" y1="-1.25" x2="1.5" y2="-1.25" width="0.127" layer="21"/>
<wire x1="1.5" y1="-1.25" x2="1.5" y2="-0.55" width="0.127" layer="21"/>
<wire x1="1.5" y1="0.55" x2="1.5" y2="1.25" width="0.127" layer="21"/>
<wire x1="1.5" y1="1.25" x2="1.3" y2="1.25" width="0.127" layer="21"/>
<text x="-2.032" y="-1.524" size="0.635" layer="21" ratio="15" rot="R90">&gt;NAME</text>
</package>
<package name="RGB_LED">
<wire x1="1.6" y1="1.4" x2="1.6" y2="-1.4" width="0.127" layer="51"/>
<wire x1="1.6" y1="-1.4" x2="-1.6" y2="-1.4" width="0.127" layer="21"/>
<wire x1="-1.6" y1="-1.4" x2="-1.6" y2="1.4" width="0.127" layer="51"/>
<wire x1="-1.6" y1="1.4" x2="1.6" y2="1.4" width="0.127" layer="21"/>
<circle x="-0.63" y="0.938" radius="0.1414" width="0.254" layer="21"/>
<smd name="4" x="1.55" y="0.75" dx="1.1" dy="0.9" layer="1"/>
<smd name="3" x="1.55" y="-0.75" dx="1.1" dy="0.9" layer="1"/>
<smd name="1" x="-1.55" y="0.75" dx="1.1" dy="0.9" layer="1"/>
<smd name="2" x="-1.55" y="-0.75" dx="1.1" dy="0.9" layer="1"/>
<text x="-1.524" y="1.524" size="0.635" layer="21" ratio="15">&gt;NAME</text>
</package>
<package name="MOTOR_PADS">
<pad name="1" x="-0.508" y="0.254" drill="0.381" diameter="0.762"/>
<pad name="2" x="0.508" y="0.254" drill="0.381" diameter="0.762"/>
<circle x="0.508" y="0.254" radius="0.508" width="0.2032" layer="21"/>
<circle x="0.508" y="0.254" radius="0.508" width="0.2032" layer="22"/>
<wire x1="-5" y1="1.45" x2="5" y2="1.45" width="0.127" layer="52"/>
<wire x1="5" y1="1.45" x2="5" y2="-1.45" width="0.127" layer="52"/>
<wire x1="5" y1="-1.45" x2="-5" y2="-1.45" width="0.127" layer="52"/>
<wire x1="-5" y1="-1.45" x2="-5" y2="1.45" width="0.127" layer="52"/>
<wire x1="-5.5" y1="2" x2="5.5" y2="2" width="1" layer="43"/>
<wire x1="5.5" y1="2" x2="5.5" y2="-2" width="1" layer="43"/>
<wire x1="5.5" y1="-2" x2="-5.5" y2="-2" width="1" layer="43"/>
<wire x1="-5.5" y1="-2" x2="-5.5" y2="2" width="1" layer="43"/>
</package>
<package name="RGB_SENSOR">
<smd name="A1" x="-1.4" y="0.7" dx="1" dy="1" layer="1"/>
<smd name="C1" x="1.4" y="0.7" dx="1" dy="1" layer="1"/>
<smd name="A2" x="-1.4" y="-0.7" dx="1" dy="1" layer="1"/>
<smd name="C2" x="1.4" y="-0.7" dx="1" dy="1" layer="1"/>
<wire x1="0.6096" y1="1.0922" x2="0.6096" y2="-1.0922" width="0.3048" layer="21"/>
<wire x1="-1.6" y1="1.35" x2="1.6" y2="1.35" width="0.127" layer="21"/>
<wire x1="-1.6" y1="-1.35" x2="1.6" y2="-1.35" width="0.127" layer="21"/>
<text x="-1.524" y="1.524" size="0.635" layer="21" ratio="15">&gt;NAME</text>
</package>
<package name="DFN-10-EJ">
<wire x1="-1.37" y1="-1.39" x2="-1.37" y2="1.41" width="0.2032" layer="21"/>
<wire x1="-1.37" y1="1.41" x2="1.43" y2="1.41" width="0.2032" layer="51"/>
<wire x1="1.43" y1="1.41" x2="1.43" y2="-1.39" width="0.2032" layer="21"/>
<wire x1="1.43" y1="-1.39" x2="-1.37" y2="-1.39" width="0.2032" layer="51"/>
<circle x="-0.72" y="-0.765" radius="0.275" width="0" layer="21"/>
<smd name="1" x="-0.97" y="-1.54" dx="0.3" dy="0.85" layer="1" roundness="25" stop="no"/>
<smd name="2" x="-0.47" y="-1.54" dx="0.3" dy="0.85" layer="1" roundness="25" stop="no"/>
<smd name="3" x="0.03" y="-1.54" dx="0.3" dy="0.85" layer="1" roundness="25" stop="no"/>
<smd name="4" x="0.53" y="-1.54" dx="0.3" dy="0.85" layer="1" roundness="25" stop="no"/>
<smd name="5" x="1.03" y="-1.54" dx="0.3" dy="0.85" layer="1" roundness="25" stop="no"/>
<smd name="6" x="1.03" y="1.56" dx="0.3" dy="0.85" layer="1" roundness="25" stop="no"/>
<smd name="7" x="0.53" y="1.56" dx="0.3" dy="0.85" layer="1" roundness="25" stop="no"/>
<smd name="8" x="0.03" y="1.56" dx="0.3" dy="0.85" layer="1" roundness="25" stop="no"/>
<smd name="9" x="-0.47" y="1.56" dx="0.3" dy="0.85" layer="1" roundness="25" stop="no"/>
<smd name="10" x="-0.97" y="1.56" dx="0.3" dy="0.85" layer="1" roundness="25" stop="no"/>
<rectangle x1="-1.145" y1="-1.99" x2="-0.795" y2="-1.09" layer="29"/>
<rectangle x1="-0.645" y1="-1.99" x2="-0.295" y2="-1.09" layer="29"/>
<rectangle x1="-0.145" y1="-1.99" x2="0.205" y2="-1.09" layer="29"/>
<rectangle x1="0.355" y1="-1.99" x2="0.705" y2="-1.09" layer="29"/>
<rectangle x1="0.855" y1="-1.99" x2="1.205" y2="-1.09" layer="29"/>
<rectangle x1="0.855" y1="1.11" x2="1.205" y2="2.01" layer="29" rot="R180"/>
<rectangle x1="0.355" y1="1.11" x2="0.705" y2="2.01" layer="29" rot="R180"/>
<rectangle x1="-0.145" y1="1.11" x2="0.205" y2="2.01" layer="29" rot="R180"/>
<rectangle x1="-0.645" y1="1.11" x2="-0.295" y2="2.01" layer="29" rot="R180"/>
<rectangle x1="-1.145" y1="1.11" x2="-0.795" y2="2.01" layer="29" rot="R180"/>
<text x="-1.524" y="-1.27" size="0.635" layer="21" ratio="15" rot="R90">&gt;NAME</text>
</package>
<package name="MLF-64-L">
<wire x1="-4.05" y1="4.4" x2="4.05" y2="4.4" width="0.254" layer="51"/>
<wire x1="4.05" y1="4.4" x2="4.4" y2="4.05" width="0.254" layer="21"/>
<wire x1="4.4" y1="4.05" x2="4.4" y2="-4.05" width="0.254" layer="51"/>
<wire x1="4.4" y1="-4.05" x2="4.05" y2="-4.4" width="0.254" layer="21"/>
<wire x1="4.05" y1="-4.4" x2="-4.05" y2="-4.4" width="0.254" layer="51"/>
<wire x1="-4.05" y1="-4.4" x2="-4.4" y2="-4.05" width="0.254" layer="21"/>
<wire x1="-4.4" y1="-4.05" x2="-4.4" y2="4.05" width="0.254" layer="51"/>
<wire x1="-4.4" y1="4.05" x2="-4.05" y2="4.4" width="0.254" layer="21"/>
<circle x="-3.4" y="3.4" radius="0.2" width="0.254" layer="21"/>
<smd name="1" x="-4.425" y="3.75" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="2" x="-4.425" y="3.25" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="3" x="-4.425" y="2.75" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="4" x="-4.425" y="2.25" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="5" x="-4.425" y="1.75" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="6" x="-4.425" y="1.25" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="7" x="-4.425" y="0.75" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="8" x="-4.425" y="0.25" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="9" x="-4.425" y="-0.25" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="10" x="-4.425" y="-0.75" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="11" x="-4.425" y="-1.25" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="12" x="-4.425" y="-1.75" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="13" x="-4.425" y="-2.25" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="14" x="-4.425" y="-2.75" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="15" x="-4.425" y="-3.25" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="16" x="-4.425" y="-3.75" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="17" x="-3.75" y="-4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="18" x="-3.25" y="-4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="19" x="-2.75" y="-4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="20" x="-2.25" y="-4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="21" x="-1.75" y="-4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="22" x="-1.25" y="-4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="23" x="-0.75" y="-4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="24" x="-0.25" y="-4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="25" x="0.25" y="-4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="26" x="0.75" y="-4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="27" x="1.25" y="-4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="28" x="1.75" y="-4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="29" x="2.25" y="-4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="30" x="2.75" y="-4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="31" x="3.25" y="-4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="32" x="3.75" y="-4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="33" x="4.425" y="-3.75" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="34" x="4.425" y="-3.25" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="35" x="4.425" y="-2.75" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="36" x="4.425" y="-2.25" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="37" x="4.425" y="-1.75" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="38" x="4.425" y="-1.25" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="39" x="4.425" y="-0.75" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="40" x="4.425" y="-0.25" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="41" x="4.425" y="0.25" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="42" x="4.425" y="0.75" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="43" x="4.425" y="1.25" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="44" x="4.425" y="1.75" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="45" x="4.425" y="2.25" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="46" x="4.425" y="2.75" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="47" x="4.425" y="3.25" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="48" x="4.425" y="3.75" dx="0.9" dy="0.3" layer="1" roundness="50"/>
<smd name="49" x="3.75" y="4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="50" x="3.25" y="4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="51" x="2.75" y="4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="52" x="2.25" y="4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="53" x="1.75" y="4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="54" x="1.25" y="4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="55" x="0.75" y="4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="56" x="0.25" y="4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="57" x="-0.25" y="4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="58" x="-0.75" y="4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="59" x="-1.25" y="4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="60" x="-1.75" y="4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="61" x="-2.25" y="4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="62" x="-2.75" y="4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="63" x="-3.25" y="4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<smd name="64" x="-3.75" y="4.425" dx="0.3" dy="0.9" layer="1" roundness="50"/>
<rectangle x1="-4.5" y1="3.625" x2="-4" y2="3.875" layer="51"/>
<rectangle x1="-4.5" y1="3.125" x2="-4" y2="3.375" layer="51"/>
<rectangle x1="-4.5" y1="2.625" x2="-4" y2="2.875" layer="51"/>
<rectangle x1="-4.5" y1="2.125" x2="-4" y2="2.375" layer="51"/>
<rectangle x1="-4.5" y1="1.625" x2="-4" y2="1.875" layer="51"/>
<rectangle x1="-4.5" y1="1.125" x2="-4" y2="1.375" layer="51"/>
<rectangle x1="-4.5" y1="0.625" x2="-4" y2="0.875" layer="51"/>
<rectangle x1="-4.5" y1="0.125" x2="-4" y2="0.375" layer="51"/>
<rectangle x1="-4.5" y1="-0.375" x2="-4" y2="-0.125" layer="51"/>
<rectangle x1="-4.5" y1="-0.875" x2="-4" y2="-0.625" layer="51"/>
<rectangle x1="-4.5" y1="-1.375" x2="-4" y2="-1.125" layer="51"/>
<rectangle x1="-4.5" y1="-1.875" x2="-4" y2="-1.625" layer="51"/>
<rectangle x1="-4.5" y1="-2.375" x2="-4" y2="-2.125" layer="51"/>
<rectangle x1="-4.5" y1="-2.875" x2="-4" y2="-2.625" layer="51"/>
<rectangle x1="-4.5" y1="-3.375" x2="-4" y2="-3.125" layer="51"/>
<rectangle x1="-4.5" y1="-3.875" x2="-4" y2="-3.625" layer="51"/>
<rectangle x1="-3.875" y1="-4.5" x2="-3.625" y2="-4" layer="51"/>
<rectangle x1="-3.375" y1="-4.5" x2="-3.125" y2="-4" layer="51"/>
<rectangle x1="-2.875" y1="-4.5" x2="-2.625" y2="-4" layer="51"/>
<rectangle x1="-2.375" y1="-4.5" x2="-2.125" y2="-4" layer="51"/>
<rectangle x1="-1.875" y1="-4.5" x2="-1.625" y2="-4" layer="51"/>
<rectangle x1="-1.375" y1="-4.5" x2="-1.125" y2="-4" layer="51"/>
<rectangle x1="-0.875" y1="-4.5" x2="-0.625" y2="-4" layer="51"/>
<rectangle x1="-0.375" y1="-4.5" x2="-0.125" y2="-4" layer="51"/>
<rectangle x1="0.125" y1="-4.5" x2="0.375" y2="-4" layer="51"/>
<rectangle x1="0.625" y1="-4.5" x2="0.875" y2="-4" layer="51"/>
<rectangle x1="1.125" y1="-4.5" x2="1.375" y2="-4" layer="51"/>
<rectangle x1="1.625" y1="-4.5" x2="1.875" y2="-4" layer="51"/>
<rectangle x1="2.125" y1="-4.5" x2="2.375" y2="-4" layer="51"/>
<rectangle x1="2.625" y1="-4.5" x2="2.875" y2="-4" layer="51"/>
<rectangle x1="3.125" y1="-4.5" x2="3.375" y2="-4" layer="51"/>
<rectangle x1="3.625" y1="-4.5" x2="3.875" y2="-4" layer="51"/>
<rectangle x1="4" y1="-3.875" x2="4.5" y2="-3.625" layer="51"/>
<rectangle x1="4" y1="-3.375" x2="4.5" y2="-3.125" layer="51"/>
<rectangle x1="4" y1="-2.875" x2="4.5" y2="-2.625" layer="51"/>
<rectangle x1="4" y1="-2.375" x2="4.5" y2="-2.125" layer="51"/>
<rectangle x1="4" y1="-1.875" x2="4.5" y2="-1.625" layer="51"/>
<rectangle x1="4" y1="-1.375" x2="4.5" y2="-1.125" layer="51"/>
<rectangle x1="4" y1="-0.875" x2="4.5" y2="-0.625" layer="51"/>
<rectangle x1="4" y1="-0.375" x2="4.5" y2="-0.125" layer="51"/>
<rectangle x1="4" y1="0.125" x2="4.5" y2="0.375" layer="51"/>
<rectangle x1="4" y1="0.625" x2="4.5" y2="0.875" layer="51"/>
<rectangle x1="4" y1="1.125" x2="4.5" y2="1.375" layer="51"/>
<rectangle x1="4" y1="1.625" x2="4.5" y2="1.875" layer="51"/>
<rectangle x1="4" y1="2.125" x2="4.5" y2="2.375" layer="51"/>
<rectangle x1="4" y1="2.625" x2="4.5" y2="2.875" layer="51"/>
<rectangle x1="4" y1="3.125" x2="4.5" y2="3.375" layer="51"/>
<rectangle x1="4" y1="3.625" x2="4.5" y2="3.875" layer="51"/>
<rectangle x1="3.625" y1="4" x2="3.875" y2="4.5" layer="51"/>
<rectangle x1="3.125" y1="4" x2="3.375" y2="4.5" layer="51"/>
<rectangle x1="2.625" y1="4" x2="2.875" y2="4.5" layer="51"/>
<rectangle x1="2.125" y1="4" x2="2.375" y2="4.5" layer="51"/>
<rectangle x1="1.625" y1="4" x2="1.875" y2="4.5" layer="51"/>
<rectangle x1="1.125" y1="4" x2="1.375" y2="4.5" layer="51"/>
<rectangle x1="0.625" y1="4" x2="0.875" y2="4.5" layer="51"/>
<rectangle x1="0.125" y1="4" x2="0.375" y2="4.5" layer="51"/>
<rectangle x1="-0.375" y1="4" x2="-0.125" y2="4.5" layer="51"/>
<rectangle x1="-0.875" y1="4" x2="-0.625" y2="4.5" layer="51"/>
<rectangle x1="-1.375" y1="4" x2="-1.125" y2="4.5" layer="51"/>
<rectangle x1="-1.875" y1="4" x2="-1.625" y2="4.5" layer="51"/>
<rectangle x1="-2.375" y1="4" x2="-2.125" y2="4.5" layer="51"/>
<rectangle x1="-2.875" y1="4" x2="-2.625" y2="4.5" layer="51"/>
<rectangle x1="-3.375" y1="4" x2="-3.125" y2="4.5" layer="51"/>
<rectangle x1="-3.875" y1="4" x2="-3.625" y2="4.5" layer="51"/>
<text x="-3.81" y="5.08" size="0.635" layer="21" ratio="15">&gt;NAME</text>
</package>
<package name="0805">
<wire x1="-0.41" y1="0.635" x2="0.41" y2="0.635" width="0.1524" layer="51"/>
<wire x1="-0.41" y1="-0.635" x2="0.41" y2="-0.635" width="0.1524" layer="51"/>
<smd name="1" x="-0.85" y="0" dx="1.3" dy="1.5" layer="1"/>
<smd name="2" x="0.85" y="0" dx="1.3" dy="1.5" layer="1"/>
<rectangle x1="0.4064" y1="-0.6985" x2="1.0564" y2="0.7015" layer="51"/>
<rectangle x1="-1.0668" y1="-0.6985" x2="-0.4168" y2="0.7015" layer="51"/>
<text x="1.6256" y="-0.3556" size="0.635" layer="21" ratio="15">&gt;NAME</text>
<wire x1="-1.4224" y1="0.8382" x2="1.4224" y2="0.8382" width="0.127" layer="21"/>
<wire x1="-1.4224" y1="-0.8382" x2="1.4224" y2="-0.8382" width="0.127" layer="21"/>
</package>
<package name="SOD-123">
<smd name="A" x="-1.635" y="0" dx="0.91" dy="1.22" layer="1"/>
<smd name="C" x="1.635" y="0" dx="0.91" dy="1.22" layer="1"/>
<wire x1="-1.3462" y1="0.8382" x2="-1.3462" y2="0.6858" width="0.127" layer="21"/>
<wire x1="-1.3462" y1="0.8382" x2="1.3462" y2="0.8382" width="0.127" layer="21"/>
<wire x1="1.3462" y1="0.8382" x2="1.3462" y2="0.6858" width="0.127" layer="21"/>
<wire x1="-1.3462" y1="-0.6858" x2="-1.3462" y2="-0.8382" width="0.127" layer="21"/>
<wire x1="-1.3462" y1="-0.8382" x2="1.3462" y2="-0.8382" width="0.127" layer="21"/>
<wire x1="1.3462" y1="-0.8382" x2="1.3462" y2="-0.6858" width="0.127" layer="21"/>
<wire x1="0.7874" y1="0.5588" x2="0.7874" y2="-0.5588" width="0.3048" layer="21"/>
<text x="2.24" y="-0.3" size="0.635" layer="21" ratio="15">&gt;NAME</text>
</package>
<package name="1210">
<text x="2.3288" y="-0.254" size="0.635" layer="21" ratio="15">&gt;NAME</text>
<wire x1="-2.473" y1="1.483" x2="2.473" y2="1.483" width="0.0508" layer="39"/>
<wire x1="2.473" y1="-1.483" x2="-2.473" y2="-1.483" width="0.0508" layer="39"/>
<wire x1="-2.473" y1="-1.483" x2="-2.473" y2="1.483" width="0.0508" layer="39"/>
<wire x1="-0.9652" y1="1.2446" x2="0.9652" y2="1.2446" width="0.1016" layer="51"/>
<wire x1="-0.9652" y1="-1.2446" x2="0.9652" y2="-1.2446" width="0.1016" layer="51"/>
<wire x1="2.473" y1="1.483" x2="2.473" y2="-1.483" width="0.0508" layer="39"/>
<smd name="1" x="-1.4" y="0" dx="1.6" dy="2.7" layer="1"/>
<smd name="2" x="1.4" y="0" dx="1.6" dy="2.7" layer="1"/>
<rectangle x1="-1.7018" y1="-1.2954" x2="-0.9517" y2="1.3045" layer="51"/>
<rectangle x1="0.9517" y1="-1.3045" x2="1.7018" y2="1.2954" layer="51"/>
<rectangle x1="-0.1999" y1="-0.4001" x2="0.1999" y2="0.4001" layer="35"/>
<wire x1="-2.1" y1="-1.5" x2="2.1" y2="-1.5" width="0.127" layer="21"/>
<wire x1="2.1" y1="1.5" x2="-2.1" y2="1.5" width="0.127" layer="21"/>
</package>
<package name="DFN-8">
<wire x1="-2.94" y1="2.41" x2="2.94" y2="2.41" width="0.127" layer="21"/>
<wire x1="2.94" y1="2.41" x2="2.94" y2="2.26" width="0.127" layer="21"/>
<wire x1="-2.94" y1="2.41" x2="-2.94" y2="2.26" width="0.127" layer="21"/>
<wire x1="2.94" y1="-2.41" x2="-2.94" y2="-2.41" width="0.127" layer="21"/>
<wire x1="-2.94" y1="-2.41" x2="-2.94" y2="-2.26" width="0.127" layer="21"/>
<wire x1="2.94" y1="-2.41" x2="2.94" y2="-2.26" width="0.127" layer="21"/>
<wire x1="-2.94" y1="2.41" x2="-2.94" y2="-2.41" width="0.127" layer="51"/>
<wire x1="2.94" y1="-2.41" x2="2.94" y2="2.41" width="0.127" layer="51"/>
<circle x="-2.02" y="1.78" radius="0.22" width="0.127" layer="21"/>
<smd name="1" x="-2.89" y="1.905" dx="0.5" dy="1" layer="1" rot="R90"/>
<smd name="2" x="-2.89" y="0.635" dx="0.5" dy="1" layer="1" rot="R90"/>
<smd name="3" x="-2.89" y="-0.635" dx="0.5" dy="1" layer="1" rot="R90"/>
<smd name="4" x="-2.89" y="-1.905" dx="0.5" dy="1" layer="1" rot="R90"/>
<smd name="5" x="2.89" y="-1.905" dx="0.5" dy="1" layer="1" rot="R270"/>
<smd name="6" x="2.89" y="-0.635" dx="0.5" dy="1" layer="1" rot="R270"/>
<smd name="7" x="2.89" y="0.635" dx="0.5" dy="1" layer="1" rot="R270"/>
<smd name="8" x="2.89" y="1.905" dx="0.5" dy="1" layer="1" rot="R270"/>
<text x="-1.143" y="2.54" size="0.635" layer="21" ratio="15">&gt;NAME</text>
</package>
<package name="SOT-363">
<wire x1="-1" y1="0.55" x2="1" y2="0.55" width="0.127" layer="51"/>
<wire x1="1" y1="0.55" x2="1" y2="-0.55" width="0.127" layer="21"/>
<wire x1="1" y1="-0.55" x2="-1" y2="-0.55" width="0.127" layer="51"/>
<wire x1="-1" y1="-0.55" x2="-1" y2="0.55" width="0.127" layer="21"/>
<circle x="-0.7" y="-0.25" radius="0.15" width="0" layer="21"/>
<smd name="1" x="-0.65" y="-0.8" dx="0.4" dy="0.8" layer="1"/>
<smd name="2" x="0" y="-0.8" dx="0.4" dy="0.8" layer="1"/>
<smd name="3" x="0.65" y="-0.8" dx="0.4" dy="0.8" layer="1"/>
<smd name="4" x="0.65" y="0.8" dx="0.4" dy="0.8" layer="1"/>
<smd name="5" x="0" y="0.8" dx="0.4" dy="0.8" layer="1"/>
<smd name="6" x="-0.65" y="0.8" dx="0.4" dy="0.8" layer="1"/>
<rectangle x1="-0.8" y1="-1.1" x2="-0.5" y2="-0.6" layer="51"/>
<rectangle x1="-0.15" y1="-1.1" x2="0.15" y2="-0.6" layer="51"/>
<rectangle x1="0.5" y1="-1.1" x2="0.8" y2="-0.6" layer="51"/>
<rectangle x1="0.5" y1="0.6" x2="0.8" y2="1.1" layer="51"/>
<rectangle x1="-0.15" y1="0.6" x2="0.15" y2="1.1" layer="51"/>
<rectangle x1="-0.8" y1="0.6" x2="-0.5" y2="1.1" layer="51"/>
<text x="-1.27" y="-1.27" size="0.635" layer="21" ratio="15" rot="R90">&gt;NAME</text>
</package>
<package name="EDGE_CONNECTOR3">
<smd name="2A" x="0" y="0" dx="0.762" dy="2.54" layer="1" roundness="100"/>
<smd name="3A" x="1.27" y="0" dx="0.762" dy="2.54" layer="1" roundness="100"/>
<smd name="1A" x="-1.27" y="0" dx="0.762" dy="2.54" layer="1" roundness="100"/>
<smd name="2B" x="0" y="0" dx="0.762" dy="2.54" layer="16" roundness="100" rot="R180"/>
<smd name="1B" x="-1.27" y="0" dx="0.762" dy="2.54" layer="16" roundness="100" rot="R180"/>
<smd name="3B" x="1.27" y="0" dx="0.762" dy="2.54" layer="16" roundness="100" rot="R180"/>
</package>
<package name="MLF-64">
<wire x1="-4.05" y1="4.4" x2="4.05" y2="4.4" width="0.254" layer="51"/>
<wire x1="4.05" y1="4.4" x2="4.4" y2="4.05" width="0.254" layer="21"/>
<wire x1="4.4" y1="4.05" x2="4.4" y2="-4.05" width="0.254" layer="51"/>
<wire x1="4.4" y1="-4.05" x2="4.05" y2="-4.4" width="0.254" layer="21"/>
<wire x1="4.05" y1="-4.4" x2="-4.05" y2="-4.4" width="0.254" layer="51"/>
<wire x1="-4.05" y1="-4.4" x2="-4.4" y2="-4.05" width="0.254" layer="21"/>
<wire x1="-4.4" y1="-4.05" x2="-4.4" y2="4.05" width="0.254" layer="51"/>
<wire x1="-4.4" y1="4.05" x2="-4.05" y2="4.4" width="0.254" layer="21"/>
<circle x="-3.4" y="3.4" radius="0.2" width="0.254" layer="21"/>
<smd name="1" x="-4.325" y="3.75" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="2" x="-4.325" y="3.25" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="3" x="-4.325" y="2.75" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="4" x="-4.325" y="2.25" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="5" x="-4.325" y="1.75" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="6" x="-4.325" y="1.25" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="7" x="-4.325" y="0.75" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="8" x="-4.325" y="0.25" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="9" x="-4.325" y="-0.25" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="10" x="-4.325" y="-0.75" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="11" x="-4.325" y="-1.25" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="12" x="-4.325" y="-1.75" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="13" x="-4.325" y="-2.25" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="14" x="-4.325" y="-2.75" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="15" x="-4.325" y="-3.25" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="16" x="-4.325" y="-3.75" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="17" x="-3.75" y="-4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="18" x="-3.25" y="-4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="19" x="-2.75" y="-4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="20" x="-2.25" y="-4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="21" x="-1.75" y="-4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="22" x="-1.25" y="-4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="23" x="-0.75" y="-4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="24" x="-0.25" y="-4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="25" x="0.25" y="-4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="26" x="0.75" y="-4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="27" x="1.25" y="-4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="28" x="1.75" y="-4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="29" x="2.25" y="-4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="30" x="2.75" y="-4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="31" x="3.25" y="-4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="32" x="3.75" y="-4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="33" x="4.325" y="-3.75" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="34" x="4.325" y="-3.25" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="35" x="4.325" y="-2.75" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="36" x="4.325" y="-2.25" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="37" x="4.325" y="-1.75" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="38" x="4.325" y="-1.25" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="39" x="4.325" y="-0.75" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="40" x="4.325" y="-0.25" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="41" x="4.325" y="0.25" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="42" x="4.325" y="0.75" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="43" x="4.325" y="1.25" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="44" x="4.325" y="1.75" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="45" x="4.325" y="2.25" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="46" x="4.325" y="2.75" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="47" x="4.325" y="3.25" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="48" x="4.325" y="3.75" dx="0.7" dy="0.3" layer="1" roundness="50"/>
<smd name="49" x="3.75" y="4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="50" x="3.25" y="4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="51" x="2.75" y="4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="52" x="2.25" y="4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="53" x="1.75" y="4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="54" x="1.25" y="4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="55" x="0.75" y="4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="56" x="0.25" y="4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="57" x="-0.25" y="4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="58" x="-0.75" y="4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="59" x="-1.25" y="4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="60" x="-1.75" y="4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="61" x="-2.25" y="4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="62" x="-2.75" y="4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="63" x="-3.25" y="4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<smd name="64" x="-3.75" y="4.325" dx="0.3" dy="0.7" layer="1" roundness="50"/>
<rectangle x1="-4.5" y1="3.625" x2="-4" y2="3.875" layer="51"/>
<rectangle x1="-4.5" y1="3.125" x2="-4" y2="3.375" layer="51"/>
<rectangle x1="-4.5" y1="2.625" x2="-4" y2="2.875" layer="51"/>
<rectangle x1="-4.5" y1="2.125" x2="-4" y2="2.375" layer="51"/>
<rectangle x1="-4.5" y1="1.625" x2="-4" y2="1.875" layer="51"/>
<rectangle x1="-4.5" y1="1.125" x2="-4" y2="1.375" layer="51"/>
<rectangle x1="-4.5" y1="0.625" x2="-4" y2="0.875" layer="51"/>
<rectangle x1="-4.5" y1="0.125" x2="-4" y2="0.375" layer="51"/>
<rectangle x1="-4.5" y1="-0.375" x2="-4" y2="-0.125" layer="51"/>
<rectangle x1="-4.5" y1="-0.875" x2="-4" y2="-0.625" layer="51"/>
<rectangle x1="-4.5" y1="-1.375" x2="-4" y2="-1.125" layer="51"/>
<rectangle x1="-4.5" y1="-1.875" x2="-4" y2="-1.625" layer="51"/>
<rectangle x1="-4.5" y1="-2.375" x2="-4" y2="-2.125" layer="51"/>
<rectangle x1="-4.5" y1="-2.875" x2="-4" y2="-2.625" layer="51"/>
<rectangle x1="-4.5" y1="-3.375" x2="-4" y2="-3.125" layer="51"/>
<rectangle x1="-4.5" y1="-3.875" x2="-4" y2="-3.625" layer="51"/>
<rectangle x1="-3.875" y1="-4.5" x2="-3.625" y2="-4" layer="51"/>
<rectangle x1="-3.375" y1="-4.5" x2="-3.125" y2="-4" layer="51"/>
<rectangle x1="-2.875" y1="-4.5" x2="-2.625" y2="-4" layer="51"/>
<rectangle x1="-2.375" y1="-4.5" x2="-2.125" y2="-4" layer="51"/>
<rectangle x1="-1.875" y1="-4.5" x2="-1.625" y2="-4" layer="51"/>
<rectangle x1="-1.375" y1="-4.5" x2="-1.125" y2="-4" layer="51"/>
<rectangle x1="-0.875" y1="-4.5" x2="-0.625" y2="-4" layer="51"/>
<rectangle x1="-0.375" y1="-4.5" x2="-0.125" y2="-4" layer="51"/>
<rectangle x1="0.125" y1="-4.5" x2="0.375" y2="-4" layer="51"/>
<rectangle x1="0.625" y1="-4.5" x2="0.875" y2="-4" layer="51"/>
<rectangle x1="1.125" y1="-4.5" x2="1.375" y2="-4" layer="51"/>
<rectangle x1="1.625" y1="-4.5" x2="1.875" y2="-4" layer="51"/>
<rectangle x1="2.125" y1="-4.5" x2="2.375" y2="-4" layer="51"/>
<rectangle x1="2.625" y1="-4.5" x2="2.875" y2="-4" layer="51"/>
<rectangle x1="3.125" y1="-4.5" x2="3.375" y2="-4" layer="51"/>
<rectangle x1="3.625" y1="-4.5" x2="3.875" y2="-4" layer="51"/>
<rectangle x1="4" y1="-3.875" x2="4.5" y2="-3.625" layer="51"/>
<rectangle x1="4" y1="-3.375" x2="4.5" y2="-3.125" layer="51"/>
<rectangle x1="4" y1="-2.875" x2="4.5" y2="-2.625" layer="51"/>
<rectangle x1="4" y1="-2.375" x2="4.5" y2="-2.125" layer="51"/>
<rectangle x1="4" y1="-1.875" x2="4.5" y2="-1.625" layer="51"/>
<rectangle x1="4" y1="-1.375" x2="4.5" y2="-1.125" layer="51"/>
<rectangle x1="4" y1="-0.875" x2="4.5" y2="-0.625" layer="51"/>
<rectangle x1="4" y1="-0.375" x2="4.5" y2="-0.125" layer="51"/>
<rectangle x1="4" y1="0.125" x2="4.5" y2="0.375" layer="51"/>
<rectangle x1="4" y1="0.625" x2="4.5" y2="0.875" layer="51"/>
<rectangle x1="4" y1="1.125" x2="4.5" y2="1.375" layer="51"/>
<rectangle x1="4" y1="1.625" x2="4.5" y2="1.875" layer="51"/>
<rectangle x1="4" y1="2.125" x2="4.5" y2="2.375" layer="51"/>
<rectangle x1="4" y1="2.625" x2="4.5" y2="2.875" layer="51"/>
<rectangle x1="4" y1="3.125" x2="4.5" y2="3.375" layer="51"/>
<rectangle x1="4" y1="3.625" x2="4.5" y2="3.875" layer="51"/>
<rectangle x1="3.625" y1="4" x2="3.875" y2="4.5" layer="51"/>
<rectangle x1="3.125" y1="4" x2="3.375" y2="4.5" layer="51"/>
<rectangle x1="2.625" y1="4" x2="2.875" y2="4.5" layer="51"/>
<rectangle x1="2.125" y1="4" x2="2.375" y2="4.5" layer="51"/>
<rectangle x1="1.625" y1="4" x2="1.875" y2="4.5" layer="51"/>
<rectangle x1="1.125" y1="4" x2="1.375" y2="4.5" layer="51"/>
<rectangle x1="0.625" y1="4" x2="0.875" y2="4.5" layer="51"/>
<rectangle x1="0.125" y1="4" x2="0.375" y2="4.5" layer="51"/>
<rectangle x1="-0.375" y1="4" x2="-0.125" y2="4.5" layer="51"/>
<rectangle x1="-0.875" y1="4" x2="-0.625" y2="4.5" layer="51"/>
<rectangle x1="-1.375" y1="4" x2="-1.125" y2="4.5" layer="51"/>
<rectangle x1="-1.875" y1="4" x2="-1.625" y2="4.5" layer="51"/>
<rectangle x1="-2.375" y1="4" x2="-2.125" y2="4.5" layer="51"/>
<rectangle x1="-2.875" y1="4" x2="-2.625" y2="4.5" layer="51"/>
<rectangle x1="-3.375" y1="4" x2="-3.125" y2="4.5" layer="51"/>
<rectangle x1="-3.875" y1="4" x2="-3.625" y2="4.5" layer="51"/>
<text x="-3.81" y="4.826" size="0.635" layer="21" ratio="15">&gt;NAME</text>
</package>
<package name="1206">
<wire x1="-0.965" y1="0.787" x2="0.965" y2="0.787" width="0.1016" layer="51"/>
<wire x1="-0.965" y1="-0.787" x2="0.965" y2="-0.787" width="0.1016" layer="51"/>
<smd name="1" x="-1.4" y="0" dx="1.6" dy="1.8" layer="1"/>
<smd name="2" x="1.4" y="0" dx="1.6" dy="1.8" layer="1"/>
<text x="2.33" y="-0.3522" size="0.635" layer="21" ratio="15">&gt;NAME</text>
<rectangle x1="-1.7018" y1="-0.8509" x2="-0.9517" y2="0.8491" layer="51"/>
<rectangle x1="0.9517" y1="-0.8491" x2="1.7018" y2="0.8509" layer="51"/>
<wire x1="-2.1" y1="1.1" x2="2.1" y2="1.1" width="0.127" layer="21"/>
<wire x1="-2.1" y1="-1.1" x2="2.1" y2="-1.1" width="0.127" layer="21"/>
</package>
<package name="VFQFN-14">
<wire x1="-1.2" y1="-0.25" x2="-1.65" y2="-0.25" width="0.4" layer="29"/>
<wire x1="-1" y1="-0.95" x2="-1" y2="-1.4" width="0.4" layer="29"/>
<wire x1="-0.5" y1="-0.95" x2="-0.5" y2="-1.4" width="0.4" layer="29"/>
<wire x1="0" y1="-0.95" x2="0" y2="-1.4" width="0.4" layer="29"/>
<wire x1="0.5" y1="-0.95" x2="0.5" y2="-1.4" width="0.4" layer="29"/>
<wire x1="1" y1="-0.95" x2="1" y2="-1.4" width="0.4" layer="29"/>
<wire x1="1.2" y1="-0.25" x2="1.65" y2="-0.25" width="0.4" layer="29"/>
<wire x1="1.2" y1="0.25" x2="1.65" y2="0.25" width="0.4" layer="29"/>
<wire x1="1" y1="0.95" x2="1" y2="1.4" width="0.4" layer="29"/>
<wire x1="0.5" y1="0.95" x2="0.5" y2="1.4" width="0.4" layer="29"/>
<wire x1="0" y1="0.95" x2="0" y2="1.4" width="0.4" layer="29"/>
<wire x1="-0.5" y1="0.95" x2="-0.5" y2="1.4" width="0.4" layer="29"/>
<wire x1="-1" y1="0.95" x2="-1" y2="1.4" width="0.4" layer="29"/>
<wire x1="-1.2" y1="0.25" x2="-1.65" y2="0.25" width="0.4" layer="29"/>
<smd name="1" x="-1.425" y="-0.25" dx="0.35" dy="0.8" layer="1" roundness="100" rot="R270" stop="no"/>
<smd name="2" x="-1" y="-1.175" dx="0.35" dy="0.8" layer="1" roundness="100" stop="no"/>
<smd name="3" x="-0.5" y="-1.175" dx="0.35" dy="0.8" layer="1" roundness="100" stop="no"/>
<smd name="4" x="0" y="-1.175" dx="0.35" dy="0.8" layer="1" roundness="100" stop="no"/>
<smd name="5" x="0.5" y="-1.175" dx="0.35" dy="0.8" layer="1" roundness="100" stop="no"/>
<smd name="6" x="1" y="-1.175" dx="0.35" dy="0.8" layer="1" roundness="100" stop="no"/>
<smd name="7" x="1.425" y="-0.25" dx="0.35" dy="0.8" layer="1" roundness="100" rot="R90" stop="no"/>
<smd name="8" x="1.425" y="0.25" dx="0.35" dy="0.8" layer="1" roundness="100" rot="R90" stop="no"/>
<smd name="9" x="1" y="1.175" dx="0.35" dy="0.8" layer="1" roundness="100" rot="R180" stop="no"/>
<smd name="10" x="0.5" y="1.175" dx="0.35" dy="0.8" layer="1" roundness="100" rot="R180" stop="no"/>
<smd name="11" x="0" y="1.175" dx="0.35" dy="0.8" layer="1" roundness="100" rot="R180" stop="no"/>
<smd name="12" x="-0.5" y="1.175" dx="0.35" dy="0.8" layer="1" roundness="100" rot="R180" stop="no"/>
<smd name="13" x="-1" y="1.175" dx="0.35" dy="0.8" layer="1" roundness="100" rot="R180" stop="no"/>
<smd name="14" x="-1.425" y="0.25" dx="0.35" dy="0.8" layer="1" roundness="100" rot="R270" stop="no"/>
<circle x="-0.6284" y="-0.3956" radius="0.1414" width="0.254" layer="21"/>
<text x="-2.032" y="-1.27" size="0.635" layer="21" ratio="15" rot="R90">&gt;NAME</text>
</package>
</packages>
<symbols>
<symbol name="XMEGA_A3">
<wire x1="30.48" y1="68.58" x2="-30.48" y2="68.58" width="0.254" layer="94"/>
<wire x1="-30.48" y1="68.58" x2="-30.48" y2="-68.58" width="0.254" layer="94"/>
<wire x1="-30.48" y1="-68.58" x2="30.48" y2="-68.58" width="0.254" layer="94"/>
<wire x1="30.48" y1="-68.58" x2="30.48" y2="68.58" width="0.254" layer="94"/>
<text x="-28.194" y="24.638" size="1.6764" layer="95">AGND</text>
<text x="-28.194" y="62.738" size="1.6764" layer="95">PDI_Data</text>
<text x="-28.194" y="65.278" size="1.6764" layer="95">/Reset_PDI_CLK</text>
<text x="-28.194" y="50.038" size="1.6764" layer="95">GND</text>
<text x="-28.194" y="47.498" size="1.6764" layer="95">GND</text>
<text x="-28.194" y="44.958" size="1.6764" layer="95">GND</text>
<text x="-28.194" y="42.418" size="1.6764" layer="95">GND</text>
<text x="-28.194" y="39.878" size="1.6764" layer="95">GND</text>
<text x="-28.194" y="29.718" size="1.6764" layer="95">AVCC</text>
<text x="-28.194" y="17.018" size="1.6764" layer="95">VCC</text>
<text x="-28.194" y="14.478" size="1.6764" layer="95">VCC</text>
<text x="-28.194" y="11.938" size="1.6764" layer="95">VCC</text>
<text x="-28.194" y="9.398" size="1.6764" layer="95">VCC</text>
<text x="-28.194" y="6.858" size="1.6764" layer="95">VCC</text>
<text x="28.194" y="66.802" size="1.6764" layer="95" rot="R180">ADC0_AC0_AREF_PA0</text>
<text x="28.194" y="64.262" size="1.6764" layer="95" rot="R180">ADC1_AC1_PA1</text>
<text x="28.194" y="61.722" size="1.6764" layer="95" rot="R180">ADC2_AC2_PA2</text>
<text x="28.194" y="59.182" size="1.6764" layer="95" rot="R180">ADC3_AC3_PA3</text>
<text x="28.194" y="56.642" size="1.6764" layer="95" rot="R180">ADC4_AC4_PA4</text>
<text x="28.194" y="54.102" size="1.6764" layer="95" rot="R180">ADC5_AC5_PA5</text>
<text x="28.194" y="51.562" size="1.6764" layer="95" rot="R180">ADC6_AC6_PA6</text>
<text x="28.194" y="49.022" size="1.6764" layer="95" rot="R180">ADC7_AC7_AC0OUT_PA7</text>
<text x="28.194" y="43.942" size="1.6764" layer="95" rot="R180">ADC0_AC0_AREF_PB0</text>
<text x="28.194" y="41.402" size="1.6764" layer="95" rot="R180">ADC1_AC1_PB1</text>
<text x="28.194" y="38.862" size="1.6764" layer="95" rot="R180">ADC2_AC2_DAC0_PB2</text>
<text x="28.194" y="36.322" size="1.6764" layer="95" rot="R180">ADC3_AC3_DAC1_PB3</text>
<text x="28.194" y="33.782" size="1.6764" layer="95" rot="R180">ADC4_AC4_TMS_PB4</text>
<text x="28.194" y="31.242" size="1.6764" layer="95" rot="R180">ADC5_AC5_TDI_PB5</text>
<text x="28.194" y="28.702" size="1.6764" layer="95" rot="R180">ADC6_AC6_TCK_PB6</text>
<text x="28.194" y="26.162" size="1.6764" layer="95" rot="R180">ADC7_AC7_AC0OUT_TDO_PB7</text>
<text x="28.194" y="21.082" size="1.6764" layer="95" rot="R180">OC0A_/OC0A_SDA_PC0</text>
<text x="28.194" y="18.542" size="1.6764" layer="95" rot="R180">OC0B_OC0A_XCK0_SCL_PC1</text>
<text x="28.194" y="16.002" size="1.6764" layer="95" rot="R180">OC0C_/OC0B_RXD0_PC2</text>
<text x="28.194" y="13.462" size="1.6764" layer="95" rot="R180">OC0D_OC0B_TXD0_PC3</text>
<text x="28.194" y="10.922" size="1.6764" layer="95" rot="R180">/OC0C_OC1A_/SS_PC4</text>
<text x="28.194" y="8.382" size="1.6764" layer="95" rot="R180">OC0C_OC1B_XCK1_MOSI_PC5</text>
<text x="28.194" y="5.842" size="1.6764" layer="95" rot="R180">/OC0D_RXD1_MISO_PC6</text>
<text x="28.194" y="3.302" size="1.6764" layer="95" rot="R180">OC0D_TXD1_SCK_CLKO_EVO_PC7</text>
<text x="28.194" y="-1.778" size="1.6764" layer="95" rot="R180">OC0A_PD0</text>
<text x="28.194" y="-4.318" size="1.6764" layer="95" rot="R180">OC0B_XCK0_PD1</text>
<text x="28.194" y="-6.858" size="1.6764" layer="95" rot="R180">OC0C_RXD0_PD2</text>
<text x="28.194" y="-9.398" size="1.6764" layer="95" rot="R180">OC0D_TXD0_PD3</text>
<text x="28.194" y="-11.938" size="1.6764" layer="95" rot="R180">OC1A_/SS_PD4</text>
<text x="28.194" y="-14.478" size="1.6764" layer="95" rot="R180">OC1B_XCK1_MOSI_PD5</text>
<text x="28.194" y="-17.018" size="1.6764" layer="95" rot="R180">RXD1_MISO_PD6</text>
<text x="28.194" y="-19.558" size="1.6764" layer="95" rot="R180">TXD1_SCK_CLKO_EVO_PD7</text>
<text x="28.194" y="-24.638" size="1.6764" layer="95" rot="R180">OC0A_SDA_PE0</text>
<text x="28.194" y="-27.178" size="1.6764" layer="95" rot="R180">OC0B_XCK0_SCL_PE1</text>
<text x="28.194" y="-29.718" size="1.6764" layer="95" rot="R180">OC0C_RXD0_PE2</text>
<text x="28.194" y="-32.258" size="1.6764" layer="95" rot="R180">OC0D_TXD0_PE3</text>
<text x="28.194" y="-34.798" size="1.6764" layer="95" rot="R180">OC1A_/SS_PE4</text>
<text x="28.194" y="-37.338" size="1.6764" layer="95" rot="R180">OC1B_XCK1_MOSI_PE5</text>
<text x="28.194" y="-39.878" size="1.6764" layer="95" rot="R180">RXD1_MISO_TOSC2_PE6</text>
<text x="28.194" y="-42.418" size="1.6764" layer="95" rot="R180">TXD1_SCK_CLKO_EVO_TOSC1_PE7</text>
<text x="28.194" y="-47.498" size="1.6764" layer="95" rot="R180">OC0A_PF0</text>
<text x="28.194" y="-50.038" size="1.6764" layer="95" rot="R180">OC0B_XCK0_PF1</text>
<text x="28.194" y="-52.578" size="1.6764" layer="95" rot="R180">OC0C_RXD0_PF2</text>
<text x="28.194" y="-55.118" size="1.6764" layer="95" rot="R180">OC0D_TXD0_PF3</text>
<text x="28.194" y="-57.658" size="1.6764" layer="95" rot="R180">PF4</text>
<text x="28.194" y="-60.198" size="1.6764" layer="95" rot="R180">PF5</text>
<text x="28.194" y="-62.738" size="1.6764" layer="95" rot="R180">PF6</text>
<text x="28.194" y="-65.278" size="1.6764" layer="95" rot="R180">PF7</text>
<text x="-28.194" y="-5.842" size="1.6764" layer="95">XTAL2_PR0</text>
<text x="-28.194" y="-8.382" size="1.6764" layer="95">XTAL1_PR1</text>
<text x="-30.48" y="69.85" size="1.6764" layer="95">&gt;Name</text>
<text x="-30.48" y="-71.12" size="1.6764" layer="96">&gt;Value</text>
<pin name="PA6" x="33.02" y="50.8" visible="pad" length="short" rot="R180"/>
<pin name="PA7" x="33.02" y="48.26" visible="pad" length="short" rot="R180"/>
<pin name="AGND" x="-33.02" y="25.4" visible="pad" length="short"/>
<pin name="PB0" x="33.02" y="43.18" visible="pad" length="short" rot="R180"/>
<pin name="PB1" x="33.02" y="40.64" visible="pad" length="short" rot="R180"/>
<pin name="PB2" x="33.02" y="38.1" visible="pad" length="short" rot="R180"/>
<pin name="PB3" x="33.02" y="35.56" visible="pad" length="short" rot="R180"/>
<pin name="PB4" x="33.02" y="33.02" visible="pad" length="short" rot="R180"/>
<pin name="PB5" x="33.02" y="30.48" visible="pad" length="short" rot="R180"/>
<pin name="PB6" x="33.02" y="27.94" visible="pad" length="short" rot="R180"/>
<pin name="PB7" x="33.02" y="25.4" visible="pad" length="short" rot="R180"/>
<pin name="GND1" x="-33.02" y="50.8" visible="pad" length="short"/>
<pin name="VCC1" x="-33.02" y="17.78" visible="pad" length="short"/>
<pin name="PC0" x="33.02" y="20.32" visible="pad" length="short" rot="R180"/>
<pin name="PC1" x="33.02" y="17.78" visible="pad" length="short" rot="R180"/>
<pin name="PC2" x="33.02" y="15.24" visible="pad" length="short" rot="R180"/>
<pin name="PC3" x="33.02" y="12.7" visible="pad" length="short" rot="R180"/>
<pin name="PC4" x="33.02" y="10.16" visible="pad" length="short" rot="R180"/>
<pin name="PC5" x="33.02" y="7.62" visible="pad" length="short" rot="R180"/>
<pin name="PC6" x="33.02" y="5.08" visible="pad" length="short" rot="R180"/>
<pin name="PC7" x="33.02" y="2.54" visible="pad" length="short" rot="R180"/>
<pin name="GND2" x="-33.02" y="48.26" visible="pad" length="short"/>
<pin name="VCC2" x="-33.02" y="15.24" visible="pad" length="short"/>
<pin name="PD0" x="33.02" y="-2.54" visible="pad" length="short" rot="R180"/>
<pin name="PD1" x="33.02" y="-5.08" visible="pad" length="short" rot="R180"/>
<pin name="PD2" x="33.02" y="-7.62" visible="pad" length="short" rot="R180"/>
<pin name="PD3" x="33.02" y="-10.16" visible="pad" length="short" rot="R180"/>
<pin name="PD4" x="33.02" y="-12.7" visible="pad" length="short" rot="R180"/>
<pin name="PD5" x="33.02" y="-15.24" visible="pad" length="short" rot="R180"/>
<pin name="PD6" x="33.02" y="-17.78" visible="pad" length="short" rot="R180"/>
<pin name="PD7" x="33.02" y="-20.32" visible="pad" length="short" rot="R180"/>
<pin name="GND3" x="-33.02" y="45.72" visible="pad" length="short"/>
<pin name="VCC3" x="-33.02" y="12.7" visible="pad" length="short"/>
<pin name="PE0" x="33.02" y="-25.4" visible="pad" length="short" rot="R180"/>
<pin name="PE1" x="33.02" y="-27.94" visible="pad" length="short" rot="R180"/>
<pin name="PE2" x="33.02" y="-30.48" visible="pad" length="short" rot="R180"/>
<pin name="PE3" x="33.02" y="-33.02" visible="pad" length="short" rot="R180"/>
<pin name="PE4" x="33.02" y="-35.56" visible="pad" length="short" rot="R180"/>
<pin name="PE5" x="33.02" y="-38.1" visible="pad" length="short" rot="R180"/>
<pin name="PE6" x="33.02" y="-40.64" visible="pad" length="short" rot="R180"/>
<pin name="PE7" x="33.02" y="-43.18" visible="pad" length="short" rot="R180"/>
<pin name="GND4" x="-33.02" y="43.18" visible="pad" length="short"/>
<pin name="VCC4" x="-33.02" y="10.16" visible="pad" length="short"/>
<pin name="PF0" x="33.02" y="-48.26" visible="pad" length="short" rot="R180"/>
<pin name="PF1" x="33.02" y="-50.8" visible="pad" length="short" rot="R180"/>
<pin name="PF2" x="33.02" y="-53.34" visible="pad" length="short" rot="R180"/>
<pin name="PF3" x="33.02" y="-55.88" visible="pad" length="short" rot="R180"/>
<pin name="PF4" x="33.02" y="-58.42" visible="pad" length="short" rot="R180"/>
<pin name="PF5" x="33.02" y="-60.96" visible="pad" length="short" rot="R180"/>
<pin name="PF6" x="33.02" y="-63.5" visible="pad" length="short" rot="R180"/>
<pin name="PF7" x="33.02" y="-66.04" visible="pad" length="short" rot="R180"/>
<pin name="GND5" x="-33.02" y="40.64" visible="pad" length="short"/>
<pin name="VCC5" x="-33.02" y="7.62" visible="pad" length="short"/>
<pin name="PDI_DATA" x="-33.02" y="63.5" visible="pad" length="short"/>
<pin name="RESET" x="-33.02" y="66.04" visible="pad" length="short"/>
<pin name="PR0" x="-33.02" y="-5.08" visible="pad" length="short"/>
<pin name="PR1" x="-33.02" y="-7.62" visible="pad" length="short"/>
<pin name="AVCC" x="-33.02" y="30.48" visible="pad" length="short"/>
<pin name="PA0" x="33.02" y="66.04" visible="pad" length="short" rot="R180"/>
<pin name="PA1" x="33.02" y="63.5" visible="pad" length="short" rot="R180"/>
<pin name="PA2" x="33.02" y="60.96" visible="pad" length="short" rot="R180"/>
<pin name="PA3" x="33.02" y="58.42" visible="pad" length="short" rot="R180"/>
<pin name="PA4" x="33.02" y="55.88" visible="pad" length="short" rot="R180"/>
<pin name="PA5" x="33.02" y="53.34" visible="pad" length="short" rot="R180"/>
</symbol>
<symbol name="PHOTODIODE">
<wire x1="1.905" y1="-1.27" x2="0" y2="1.27" width="0.254" layer="94"/>
<wire x1="0" y1="1.27" x2="-1.905" y2="-1.27" width="0.254" layer="94"/>
<wire x1="-1.905" y1="-1.27" x2="1.905" y2="-1.27" width="0.254" layer="94"/>
<wire x1="-1.905" y1="1.397" x2="1.905" y2="1.397" width="0.254" layer="94"/>
<wire x1="-3.556" y1="1.016" x2="-4.572" y2="2.032" width="0.1524" layer="94"/>
<wire x1="-2.794" y1="1.778" x2="-3.81" y2="2.794" width="0.1524" layer="94"/>
<text x="3.81" y="1.27" size="1.778" layer="95">&gt;NAME</text>
<pin name="A" x="0" y="-2.54" visible="off" length="short" direction="pas" rot="R90"/>
<pin name="C" x="0" y="2.54" visible="off" length="short" direction="pas" rot="R270"/>
<polygon width="0.1524" layer="94">
<vertex x="-4.064" y="1.016"/>
<vertex x="-3.556" y="1.524"/>
<vertex x="-3.302" y="0.762"/>
</polygon>
<polygon width="0.1524" layer="94">
<vertex x="-3.302" y="1.778"/>
<vertex x="-2.794" y="2.286"/>
<vertex x="-2.54" y="1.524"/>
</polygon>
</symbol>
<symbol name="CAPACITOR">
<wire x1="0" y1="2.54" x2="0" y2="2.032" width="0.1524" layer="94"/>
<wire x1="0" y1="0" x2="0" y2="0.508" width="0.1524" layer="94"/>
<text x="1.524" y="2.921" size="1.778" layer="95">&gt;NAME</text>
<text x="1.524" y="-2.159" size="1.778" layer="96">&gt;VALUE</text>
<rectangle x1="-2.032" y1="0.508" x2="2.032" y2="1.016" layer="94"/>
<rectangle x1="-2.032" y1="1.524" x2="2.032" y2="2.032" layer="94"/>
<pin name="1" x="0" y="5.08" visible="off" length="short" direction="pas" swaplevel="1" rot="R270"/>
<pin name="2" x="0" y="-2.54" visible="off" length="short" direction="pas" swaplevel="1" rot="R90"/>
</symbol>
<symbol name="LEG">
<wire x1="-2.54" y1="2.54" x2="2.54" y2="2.54" width="0.254" layer="94"/>
<wire x1="2.54" y1="2.54" x2="0" y2="-12.7" width="0.254" layer="94"/>
<wire x1="0" y1="-12.7" x2="-2.54" y2="2.54" width="0.254" layer="94"/>
<pin name="1" x="0" y="10.16" visible="off" rot="R270"/>
</symbol>
<symbol name="DIODE">
<wire x1="-1.27" y1="-1.27" x2="1.27" y2="0" width="0.254" layer="94"/>
<wire x1="1.27" y1="0" x2="-1.27" y2="1.27" width="0.254" layer="94"/>
<wire x1="1.27" y1="1.27" x2="1.27" y2="0" width="0.254" layer="94"/>
<wire x1="-1.27" y1="1.27" x2="-1.27" y2="-1.27" width="0.254" layer="94"/>
<wire x1="1.27" y1="0" x2="1.27" y2="-1.27" width="0.254" layer="94"/>
<pin name="A" x="-2.54" y="0" visible="off" length="short" direction="pas"/>
<pin name="C" x="2.54" y="0" visible="off" length="short" direction="pas" rot="R180"/>
</symbol>
<symbol name="CAPACITOR_POLERIZED">
<wire x1="-1.524" y1="-0.889" x2="1.524" y2="-0.889" width="0.254" layer="94"/>
<wire x1="1.524" y1="-0.889" x2="1.524" y2="0" width="0.254" layer="94"/>
<wire x1="-1.524" y1="0" x2="-1.524" y2="-0.889" width="0.254" layer="94"/>
<wire x1="-1.524" y1="0" x2="1.524" y2="0" width="0.254" layer="94"/>
<text x="1.143" y="0.4826" size="1.778" layer="95">&gt;NAME</text>
<text x="-0.5842" y="0.4064" size="1.27" layer="94" rot="R90">+</text>
<text x="1.143" y="-4.5974" size="1.778" layer="96">&gt;VALUE</text>
<rectangle x1="-1.651" y1="-2.54" x2="1.651" y2="-1.651" layer="94"/>
<pin name="-" x="0" y="-5.08" visible="off" length="short" direction="pas" rot="R90"/>
<pin name="+" x="0" y="2.54" visible="off" length="short" direction="pas" rot="R270"/>
</symbol>
<symbol name="REGULATOR">
<wire x1="-7.62" y1="7.62" x2="7.62" y2="7.62" width="0.254" layer="94"/>
<wire x1="7.62" y1="7.62" x2="7.62" y2="-7.62" width="0.254" layer="94"/>
<wire x1="7.62" y1="-7.62" x2="-7.62" y2="-7.62" width="0.254" layer="94"/>
<wire x1="-7.62" y1="-7.62" x2="-7.62" y2="7.62" width="0.254" layer="94"/>
<pin name="VIN" x="-10.16" y="5.08" visible="pin" length="short"/>
<pin name="VSS" x="-10.16" y="-5.08" visible="pin" length="short"/>
<pin name="CE" x="-10.16" y="0" visible="pin" length="short"/>
<pin name="VOUT" x="10.16" y="5.08" visible="pin" length="short" rot="R180"/>
<text x="-7.62" y="8.1026" size="1.778" layer="95">&gt;NAME</text>
<text x="-7.62" y="-9.9314" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="INDUCTOR">
<wire x1="-5.08" y1="0" x2="-3.81" y2="1.27" width="0.254" layer="94" curve="-90" cap="flat"/>
<wire x1="-2.54" y1="0" x2="-3.81" y2="1.27" width="0.254" layer="94" curve="90" cap="flat"/>
<wire x1="-2.54" y1="0" x2="-1.27" y2="1.27" width="0.254" layer="94" curve="-90" cap="flat"/>
<wire x1="0" y1="0" x2="-1.27" y2="1.27" width="0.254" layer="94" curve="90" cap="flat"/>
<wire x1="0" y1="0" x2="1.27" y2="1.27" width="0.254" layer="94" curve="-90" cap="flat"/>
<wire x1="2.54" y1="0" x2="1.27" y2="1.27" width="0.254" layer="94" curve="90" cap="flat"/>
<wire x1="2.54" y1="0" x2="3.81" y2="1.27" width="0.254" layer="94" curve="-90" cap="flat"/>
<wire x1="5.08" y1="0" x2="3.81" y2="1.27" width="0.254" layer="94" curve="90" cap="flat"/>
<text x="2.54" y="3.81" size="1.778" layer="95" rot="R180">&gt;NAME</text>
<text x="5.08" y="-1.27" size="1.778" layer="96" rot="R180">&gt;VALUE</text>
<pin name="2" x="7.62" y="0" visible="off" length="short" direction="pas" swaplevel="1" rot="R180"/>
<pin name="1" x="-7.62" y="0" visible="off" length="short" direction="pas" swaplevel="1"/>
</symbol>
<symbol name="REGULATOR_5V">
<pin name="EN" x="-10.16" y="2.54" length="short"/>
<pin name="VIN" x="-10.16" y="7.62" length="short"/>
<pin name="GND" x="-10.16" y="-7.62" length="short"/>
<pin name="SW" x="-10.16" y="-2.54" length="short"/>
<pin name="VOUT" x="10.16" y="7.62" length="short" rot="R180"/>
<pin name="VFB" x="10.16" y="-7.62" length="short" rot="R180"/>
<wire x1="-7.62" y1="10.16" x2="-7.62" y2="-10.16" width="0.254" layer="94"/>
<wire x1="-7.62" y1="-10.16" x2="7.62" y2="-10.16" width="0.254" layer="94"/>
<wire x1="7.62" y1="-10.16" x2="7.62" y2="10.16" width="0.254" layer="94"/>
<wire x1="7.62" y1="10.16" x2="-7.62" y2="10.16" width="0.254" layer="94"/>
<text x="-7.62" y="11.43" size="1.778" layer="95">&gt;NAME</text>
<text x="-7.62" y="-12.7" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="RESISTOR">
<wire x1="-2.54" y1="0" x2="-2.159" y2="1.016" width="0.2032" layer="94"/>
<wire x1="-2.159" y1="1.016" x2="-1.524" y2="-1.016" width="0.2032" layer="94"/>
<wire x1="-1.524" y1="-1.016" x2="-0.889" y2="1.016" width="0.2032" layer="94"/>
<wire x1="-0.889" y1="1.016" x2="-0.254" y2="-1.016" width="0.2032" layer="94"/>
<wire x1="-0.254" y1="-1.016" x2="0.381" y2="1.016" width="0.2032" layer="94"/>
<wire x1="0.381" y1="1.016" x2="1.016" y2="-1.016" width="0.2032" layer="94"/>
<wire x1="1.016" y1="-1.016" x2="1.651" y2="1.016" width="0.2032" layer="94"/>
<wire x1="1.651" y1="1.016" x2="2.286" y2="-1.016" width="0.2032" layer="94"/>
<wire x1="2.286" y1="-1.016" x2="2.54" y2="0" width="0.2032" layer="94"/>
<text x="-3.81" y="1.4986" size="1.778" layer="95">&gt;NAME</text>
<text x="-3.81" y="-3.302" size="1.778" layer="96">&gt;VALUE</text>
<pin name="2" x="5.08" y="0" visible="off" length="short" direction="pas" swaplevel="1" rot="R180"/>
<pin name="1" x="-5.08" y="0" visible="off" length="short" direction="pas" swaplevel="1"/>
</symbol>
<symbol name="LED">
<wire x1="1.27" y1="0" x2="0" y2="-2.54" width="0.254" layer="94"/>
<wire x1="0" y1="-2.54" x2="-1.27" y2="0" width="0.254" layer="94"/>
<wire x1="1.27" y1="-2.54" x2="0" y2="-2.54" width="0.254" layer="94"/>
<wire x1="0" y1="-2.54" x2="-1.27" y2="-2.54" width="0.254" layer="94"/>
<wire x1="1.27" y1="0" x2="0" y2="0" width="0.254" layer="94"/>
<wire x1="0" y1="0" x2="-1.27" y2="0" width="0.254" layer="94"/>
<wire x1="0" y1="0" x2="0" y2="-2.54" width="0.1524" layer="94"/>
<wire x1="-2.032" y1="-0.762" x2="-3.429" y2="-2.159" width="0.1524" layer="94"/>
<wire x1="-1.905" y1="-1.905" x2="-3.302" y2="-3.302" width="0.1524" layer="94"/>
<text x="3.556" y="-4.572" size="1.778" layer="95" rot="R90">&gt;NAME</text>
<text x="5.715" y="-4.572" size="1.778" layer="96" rot="R90">&gt;VALUE</text>
<pin name="C" x="0" y="-5.08" visible="off" length="short" direction="pas" rot="R90"/>
<pin name="A" x="0" y="2.54" visible="off" length="short" direction="pas" rot="R270"/>
<polygon width="0.1524" layer="94">
<vertex x="-3.429" y="-2.159"/>
<vertex x="-3.048" y="-1.27"/>
<vertex x="-2.54" y="-1.778"/>
</polygon>
<polygon width="0.1524" layer="94">
<vertex x="-3.302" y="-3.302"/>
<vertex x="-2.921" y="-2.413"/>
<vertex x="-2.413" y="-2.921"/>
</polygon>
</symbol>
<symbol name="AND_GATE">
<wire x1="-7.62" y1="5.08" x2="-7.62" y2="-5.08" width="0.4064" layer="94"/>
<wire x1="-7.62" y1="-5.08" x2="2.54" y2="-5.08" width="0.4064" layer="94"/>
<wire x1="2.54" y1="5.08" x2="2.54" y2="-5.08" width="0.4064" layer="94" curve="-180"/>
<wire x1="2.54" y1="5.08" x2="-7.62" y2="5.08" width="0.4064" layer="94"/>
<text x="-7.62" y="5.715" size="1.778" layer="95">&gt;NAME</text>
<pin name="I0" x="-10.16" y="2.54" visible="pad" length="short" direction="in" swaplevel="1"/>
<pin name="I1" x="-10.16" y="-2.54" visible="pad" length="short" direction="in" swaplevel="1"/>
<pin name="O" x="10.16" y="0" visible="pad" length="short" direction="out" rot="R180"/>
</symbol>
<symbol name="VAR_RESISTOR">
<wire x1="-2.54" y1="0" x2="-2.159" y2="1.016" width="0.2032" layer="94"/>
<wire x1="-2.159" y1="1.016" x2="-1.524" y2="-1.016" width="0.2032" layer="94"/>
<wire x1="-1.524" y1="-1.016" x2="-0.889" y2="1.016" width="0.2032" layer="94"/>
<wire x1="-0.889" y1="1.016" x2="-0.254" y2="-1.016" width="0.2032" layer="94"/>
<wire x1="-0.254" y1="-1.016" x2="0.381" y2="1.016" width="0.2032" layer="94"/>
<wire x1="0.381" y1="1.016" x2="1.016" y2="-1.016" width="0.2032" layer="94"/>
<wire x1="1.016" y1="-1.016" x2="1.651" y2="1.016" width="0.2032" layer="94"/>
<wire x1="1.651" y1="1.016" x2="2.286" y2="-1.016" width="0.2032" layer="94"/>
<wire x1="2.286" y1="-1.016" x2="2.54" y2="0" width="0.2032" layer="94"/>
<text x="-3.556" y="4.2926" size="1.778" layer="95">&gt;NAME</text>
<pin name="2" x="5.08" y="0" visible="off" length="short" direction="pas" swaplevel="1" rot="R180"/>
<pin name="1" x="-5.08" y="0" visible="off" length="short" direction="pas" swaplevel="1"/>
<wire x1="-2.54" y1="-3.81" x2="2.54" y2="3.81" width="0.2032" layer="94"/>
<polygon width="0.254" layer="94">
<vertex x="2.54" y="3.81"/>
<vertex x="1.27" y="2.794"/>
<vertex x="2.032" y="2.286"/>
</polygon>
</symbol>
<symbol name="RGB_LED">
<wire x1="0" y1="1.27" x2="0" y2="0" width="0.254" layer="94"/>
<wire x1="0" y1="0" x2="0" y2="-1.27" width="0.254" layer="94"/>
<wire x1="0" y1="-1.27" x2="2.54" y2="0" width="0.254" layer="94"/>
<wire x1="2.54" y1="0" x2="0" y2="1.27" width="0.254" layer="94"/>
<wire x1="2.54" y1="1.27" x2="2.54" y2="-1.27" width="0.254" layer="94"/>
<wire x1="0" y1="8.89" x2="0" y2="7.62" width="0.254" layer="94"/>
<wire x1="0" y1="7.62" x2="0" y2="6.35" width="0.254" layer="94"/>
<wire x1="0" y1="6.35" x2="2.54" y2="7.62" width="0.254" layer="94"/>
<wire x1="2.54" y1="7.62" x2="0" y2="8.89" width="0.254" layer="94"/>
<wire x1="2.54" y1="8.89" x2="2.54" y2="6.35" width="0.254" layer="94"/>
<wire x1="0" y1="-6.35" x2="0" y2="-7.62" width="0.254" layer="94"/>
<wire x1="0" y1="-7.62" x2="0" y2="-8.89" width="0.254" layer="94"/>
<wire x1="0" y1="-8.89" x2="2.54" y2="-7.62" width="0.254" layer="94"/>
<wire x1="2.54" y1="-7.62" x2="0" y2="-6.35" width="0.254" layer="94"/>
<wire x1="2.54" y1="-6.35" x2="2.54" y2="-8.89" width="0.254" layer="94"/>
<wire x1="-2.54" y1="0" x2="0" y2="0" width="0.1524" layer="94"/>
<wire x1="-2.54" y1="0" x2="-2.54" y2="7.62" width="0.254" layer="94"/>
<wire x1="-2.54" y1="7.62" x2="0" y2="7.62" width="0.254" layer="94"/>
<wire x1="-2.54" y1="0" x2="-2.54" y2="-7.62" width="0.254" layer="94"/>
<wire x1="-2.54" y1="-7.62" x2="0" y2="-7.62" width="0.254" layer="94"/>
<pin name="ANODE" x="-7.62" y="0" visible="off" length="middle"/>
<pin name="R" x="7.62" y="7.62" visible="off" length="middle" rot="R180"/>
<pin name="G" x="7.62" y="0" visible="off" length="middle" rot="R180"/>
<pin name="B" x="7.62" y="-7.62" visible="off" length="middle" rot="R180"/>
<wire x1="2.794" y1="9.652" x2="4.191" y2="11.049" width="0.1524" layer="94"/>
<wire x1="2.667" y1="10.795" x2="4.064" y2="12.192" width="0.1524" layer="94"/>
<polygon width="0.1524" layer="94">
<vertex x="4.191" y="11.049"/>
<vertex x="3.81" y="10.16"/>
<vertex x="3.302" y="10.668"/>
</polygon>
<polygon width="0.1524" layer="94">
<vertex x="4.064" y="12.192"/>
<vertex x="3.683" y="11.303"/>
<vertex x="3.175" y="11.811"/>
</polygon>
<text x="4.572" y="9.398" size="1.27" layer="94">R</text>
<text x="4.572" y="1.778" size="1.27" layer="94">G</text>
<text x="4.572" y="-5.842" size="1.27" layer="94">B</text>
<wire x1="2.794" y1="2.032" x2="4.191" y2="3.429" width="0.1524" layer="94"/>
<wire x1="2.667" y1="3.175" x2="4.064" y2="4.572" width="0.1524" layer="94"/>
<polygon width="0.1524" layer="94">
<vertex x="4.191" y="3.429"/>
<vertex x="3.81" y="2.54"/>
<vertex x="3.302" y="3.048"/>
</polygon>
<polygon width="0.1524" layer="94">
<vertex x="4.064" y="4.572"/>
<vertex x="3.683" y="3.683"/>
<vertex x="3.175" y="4.191"/>
</polygon>
<wire x1="2.794" y1="-5.588" x2="4.191" y2="-4.191" width="0.1524" layer="94"/>
<wire x1="2.667" y1="-4.445" x2="4.064" y2="-3.048" width="0.1524" layer="94"/>
<polygon width="0.1524" layer="94">
<vertex x="4.191" y="-4.191"/>
<vertex x="3.81" y="-5.08"/>
<vertex x="3.302" y="-4.572"/>
</polygon>
<polygon width="0.1524" layer="94">
<vertex x="4.064" y="-3.048"/>
<vertex x="3.683" y="-3.937"/>
<vertex x="3.175" y="-3.429"/>
</polygon>
</symbol>
<symbol name="MOTOR">
<text x="-0.635" y="-0.635" size="1.778" layer="94">M</text>
<circle x="0" y="0" radius="2.54" width="0.254" layer="94"/>
<pin name="1" x="0" y="5.08" visible="off" length="short" swaplevel="1" rot="R270"/>
<pin name="2" x="0" y="-5.08" visible="off" length="short" swaplevel="1" rot="R90"/>
</symbol>
<symbol name="HALF-MOTOR">
<pin name="IN1" x="-7.62" y="5.08" visible="pad" length="short" swaplevel="1"/>
<pin name="IN2" x="-7.62" y="-5.08" visible="pad" length="short" swaplevel="1"/>
<pin name="OUT1" x="7.62" y="5.08" visible="pad" length="short" swaplevel="2" rot="R180"/>
<pin name="OUT2" x="7.62" y="-5.08" visible="pad" length="short" swaplevel="2" rot="R180"/>
<wire x1="-5.08" y1="7.62" x2="-5.08" y2="5.08" width="0.254" layer="94"/>
<wire x1="-5.08" y1="5.08" x2="-5.08" y2="-5.08" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-5.08" x2="-5.08" y2="-7.62" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-7.62" x2="5.08" y2="-7.62" width="0.254" layer="94"/>
<wire x1="5.08" y1="-7.62" x2="5.08" y2="-5.08" width="0.254" layer="94"/>
<wire x1="5.08" y1="-5.08" x2="5.08" y2="5.08" width="0.254" layer="94"/>
<wire x1="5.08" y1="5.08" x2="5.08" y2="7.62" width="0.254" layer="94"/>
<wire x1="5.08" y1="7.62" x2="-5.08" y2="7.62" width="0.254" layer="94"/>
<wire x1="-5.08" y1="5.08" x2="-2.54" y2="5.08" width="0.4064" layer="94"/>
<wire x1="-2.54" y1="5.08" x2="2.54" y2="5.08" width="0.254" layer="94"/>
<wire x1="2.54" y1="5.08" x2="5.08" y2="5.08" width="0.4064" layer="94"/>
<wire x1="-5.08" y1="-5.08" x2="-2.54" y2="-5.08" width="0.4064" layer="94"/>
<wire x1="5.08" y1="-5.08" x2="2.54" y2="-5.08" width="0.4064" layer="94"/>
<wire x1="2.54" y1="-5.08" x2="-2.54" y2="-5.08" width="0.254" layer="94"/>
<wire x1="-2.54" y1="-5.08" x2="2.54" y2="5.08" width="0.254" layer="94"/>
<wire x1="-2.54" y1="5.08" x2="2.54" y2="-5.08" width="0.254" layer="94"/>
<polygon width="0.254" layer="94">
<vertex x="-3.81" y="5.08"/>
<vertex x="-5.08" y="6.35"/>
<vertex x="-5.08" y="3.81"/>
</polygon>
<polygon width="0.254" layer="94">
<vertex x="5.08" y="5.08"/>
<vertex x="3.81" y="6.35"/>
<vertex x="3.81" y="3.81"/>
</polygon>
<polygon width="0.254" layer="94">
<vertex x="-5.08" y="-3.81"/>
<vertex x="-3.81" y="-5.08"/>
<vertex x="-5.08" y="-6.35"/>
</polygon>
<polygon width="0.254" layer="94">
<vertex x="3.81" y="-3.81"/>
<vertex x="3.81" y="-6.35"/>
<vertex x="5.08" y="-5.08"/>
</polygon>
</symbol>
<symbol name="SUPPLY">
<pin name="1" x="-5.08" y="0" visible="off" length="middle"/>
<text x="-5.08" y="1.27" size="1.778" layer="95">&gt;NAME</text>
</symbol>
<symbol name="MRAM">
<pin name="!CS" x="-10.16" y="2.54" length="short"/>
<pin name="SI" x="-10.16" y="0" length="short"/>
<pin name="SO" x="-10.16" y="-2.54" length="short"/>
<pin name="SCK" x="-10.16" y="-5.08" length="short"/>
<pin name="VDD" x="10.16" y="2.54" length="short" rot="R180"/>
<pin name="VSS" x="10.16" y="0" length="short" rot="R180"/>
<pin name="!WP" x="10.16" y="-5.08" length="short" rot="R180"/>
<pin name="!HOLD" x="10.16" y="-2.54" length="short" rot="R180"/>
<wire x1="-7.62" y1="5.08" x2="-7.62" y2="-7.62" width="0.254" layer="94"/>
<wire x1="-7.62" y1="-7.62" x2="7.62" y2="-7.62" width="0.254" layer="94"/>
<wire x1="7.62" y1="-7.62" x2="7.62" y2="5.08" width="0.254" layer="94"/>
<wire x1="7.62" y1="5.08" x2="-7.62" y2="5.08" width="0.254" layer="94"/>
</symbol>
<symbol name="N-CHANNEL_MOSFET">
<wire x1="0.762" y1="0.762" x2="0.762" y2="0" width="0.254" layer="94"/>
<wire x1="0.762" y1="0" x2="0.762" y2="-0.762" width="0.254" layer="94"/>
<wire x1="0.762" y1="3.175" x2="0.762" y2="2.54" width="0.254" layer="94"/>
<wire x1="0.762" y1="2.54" x2="0.762" y2="1.905" width="0.254" layer="94"/>
<wire x1="0.762" y1="0" x2="2.54" y2="0" width="0.1524" layer="94"/>
<wire x1="2.54" y1="0" x2="2.54" y2="-2.54" width="0.1524" layer="94"/>
<wire x1="0.762" y1="-1.905" x2="0.762" y2="-2.54" width="0.254" layer="94"/>
<wire x1="0.762" y1="-2.54" x2="0.762" y2="-3.175" width="0.254" layer="94"/>
<wire x1="0" y1="2.54" x2="0" y2="-2.54" width="0.254" layer="94"/>
<wire x1="2.54" y1="-2.54" x2="0.762" y2="-2.54" width="0.1524" layer="94"/>
<wire x1="3.81" y1="2.54" x2="3.81" y2="0.508" width="0.1524" layer="94"/>
<wire x1="3.81" y1="0.508" x2="3.81" y2="-2.54" width="0.1524" layer="94"/>
<wire x1="2.54" y1="-2.54" x2="3.81" y2="-2.54" width="0.1524" layer="94"/>
<wire x1="0.762" y1="2.54" x2="3.81" y2="2.54" width="0.1524" layer="94"/>
<wire x1="4.572" y1="0.762" x2="4.318" y2="0.508" width="0.1524" layer="94"/>
<wire x1="4.318" y1="0.508" x2="3.81" y2="0.508" width="0.1524" layer="94"/>
<wire x1="3.81" y1="0.508" x2="3.302" y2="0.508" width="0.1524" layer="94"/>
<wire x1="3.302" y1="0.508" x2="3.048" y2="0.254" width="0.1524" layer="94"/>
<circle x="2.54" y="-2.54" radius="0.3592" width="0" layer="94"/>
<circle x="2.54" y="2.54" radius="0.3592" width="0" layer="94"/>
<text x="6.35" y="0" size="1.778" layer="95">&gt;NAME</text>
<pin name="S" x="2.54" y="-5.08" visible="off" length="short" direction="pas" rot="R90"/>
<pin name="G" x="-2.54" y="-2.54" visible="off" length="short" direction="pas"/>
<pin name="D" x="2.54" y="5.08" visible="off" length="short" direction="pas" rot="R270"/>
<polygon width="0.1524" layer="94">
<vertex x="3.81" y="0.508"/>
<vertex x="3.302" y="-0.254"/>
<vertex x="4.318" y="-0.254"/>
</polygon>
<polygon width="0.1524" layer="94">
<vertex x="1.016" y="0"/>
<vertex x="2.032" y="0.762"/>
<vertex x="2.032" y="-0.762"/>
</polygon>
</symbol>
<symbol name="EDGE_PORT_3">
<pin name="2" x="-2.54" y="0" visible="off" length="short"/>
<pin name="3" x="-2.54" y="-2.54" visible="off" length="short"/>
<pin name="1" x="-2.54" y="2.54" visible="off" length="short"/>
<wire x1="0" y1="5.08" x2="0" y2="-5.08" width="0.254" layer="94"/>
<wire x1="0" y1="-5.08" x2="2.54" y2="-5.08" width="0.254" layer="94"/>
<wire x1="2.54" y1="-5.08" x2="2.54" y2="5.08" width="0.254" layer="94"/>
<wire x1="2.54" y1="5.08" x2="0" y2="5.08" width="0.254" layer="94"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="XMEGA_A3">
<gates>
<gate name="G$1" symbol="XMEGA_A3" x="0" y="0"/>
</gates>
<devices>
<device name="" package="MLF-64">
<connects>
<connect gate="G$1" pin="AGND" pad="60"/>
<connect gate="G$1" pin="AVCC" pad="61"/>
<connect gate="G$1" pin="GND1" pad="14"/>
<connect gate="G$1" pin="GND2" pad="24"/>
<connect gate="G$1" pin="GND3" pad="34"/>
<connect gate="G$1" pin="GND4" pad="44"/>
<connect gate="G$1" pin="GND5" pad="52"/>
<connect gate="G$1" pin="PA0" pad="62"/>
<connect gate="G$1" pin="PA1" pad="63"/>
<connect gate="G$1" pin="PA2" pad="64"/>
<connect gate="G$1" pin="PA3" pad="1"/>
<connect gate="G$1" pin="PA4" pad="2"/>
<connect gate="G$1" pin="PA5" pad="3"/>
<connect gate="G$1" pin="PA6" pad="4"/>
<connect gate="G$1" pin="PA7" pad="5"/>
<connect gate="G$1" pin="PB0" pad="6"/>
<connect gate="G$1" pin="PB1" pad="7"/>
<connect gate="G$1" pin="PB2" pad="8"/>
<connect gate="G$1" pin="PB3" pad="9"/>
<connect gate="G$1" pin="PB4" pad="10"/>
<connect gate="G$1" pin="PB5" pad="11"/>
<connect gate="G$1" pin="PB6" pad="12"/>
<connect gate="G$1" pin="PB7" pad="13"/>
<connect gate="G$1" pin="PC0" pad="16"/>
<connect gate="G$1" pin="PC1" pad="17"/>
<connect gate="G$1" pin="PC2" pad="18"/>
<connect gate="G$1" pin="PC3" pad="19"/>
<connect gate="G$1" pin="PC4" pad="20"/>
<connect gate="G$1" pin="PC5" pad="21"/>
<connect gate="G$1" pin="PC6" pad="22"/>
<connect gate="G$1" pin="PC7" pad="23"/>
<connect gate="G$1" pin="PD0" pad="26"/>
<connect gate="G$1" pin="PD1" pad="27"/>
<connect gate="G$1" pin="PD2" pad="28"/>
<connect gate="G$1" pin="PD3" pad="29"/>
<connect gate="G$1" pin="PD4" pad="30"/>
<connect gate="G$1" pin="PD5" pad="31"/>
<connect gate="G$1" pin="PD6" pad="32"/>
<connect gate="G$1" pin="PD7" pad="33"/>
<connect gate="G$1" pin="PDI_DATA" pad="56"/>
<connect gate="G$1" pin="PE0" pad="36"/>
<connect gate="G$1" pin="PE1" pad="37"/>
<connect gate="G$1" pin="PE2" pad="38"/>
<connect gate="G$1" pin="PE3" pad="39"/>
<connect gate="G$1" pin="PE4" pad="40"/>
<connect gate="G$1" pin="PE5" pad="41"/>
<connect gate="G$1" pin="PE6" pad="42"/>
<connect gate="G$1" pin="PE7" pad="43"/>
<connect gate="G$1" pin="PF0" pad="46"/>
<connect gate="G$1" pin="PF1" pad="47"/>
<connect gate="G$1" pin="PF2" pad="48"/>
<connect gate="G$1" pin="PF3" pad="49"/>
<connect gate="G$1" pin="PF4" pad="50"/>
<connect gate="G$1" pin="PF5" pad="51"/>
<connect gate="G$1" pin="PF6" pad="54"/>
<connect gate="G$1" pin="PF7" pad="55"/>
<connect gate="G$1" pin="PR0" pad="58"/>
<connect gate="G$1" pin="PR1" pad="59"/>
<connect gate="G$1" pin="RESET" pad="57"/>
<connect gate="G$1" pin="VCC1" pad="15"/>
<connect gate="G$1" pin="VCC2" pad="25"/>
<connect gate="G$1" pin="VCC3" pad="35"/>
<connect gate="G$1" pin="VCC4" pad="45"/>
<connect gate="G$1" pin="VCC5" pad="53"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="L" package="MLF-64-L">
<connects>
<connect gate="G$1" pin="AGND" pad="60"/>
<connect gate="G$1" pin="AVCC" pad="61"/>
<connect gate="G$1" pin="GND1" pad="14"/>
<connect gate="G$1" pin="GND2" pad="24"/>
<connect gate="G$1" pin="GND3" pad="34"/>
<connect gate="G$1" pin="GND4" pad="44"/>
<connect gate="G$1" pin="GND5" pad="52"/>
<connect gate="G$1" pin="PA0" pad="62"/>
<connect gate="G$1" pin="PA1" pad="63"/>
<connect gate="G$1" pin="PA2" pad="64"/>
<connect gate="G$1" pin="PA3" pad="1"/>
<connect gate="G$1" pin="PA4" pad="2"/>
<connect gate="G$1" pin="PA5" pad="3"/>
<connect gate="G$1" pin="PA6" pad="4"/>
<connect gate="G$1" pin="PA7" pad="5"/>
<connect gate="G$1" pin="PB0" pad="6"/>
<connect gate="G$1" pin="PB1" pad="7"/>
<connect gate="G$1" pin="PB2" pad="8"/>
<connect gate="G$1" pin="PB3" pad="9"/>
<connect gate="G$1" pin="PB4" pad="10"/>
<connect gate="G$1" pin="PB5" pad="11"/>
<connect gate="G$1" pin="PB6" pad="12"/>
<connect gate="G$1" pin="PB7" pad="13"/>
<connect gate="G$1" pin="PC0" pad="16"/>
<connect gate="G$1" pin="PC1" pad="17"/>
<connect gate="G$1" pin="PC2" pad="18"/>
<connect gate="G$1" pin="PC3" pad="19"/>
<connect gate="G$1" pin="PC4" pad="20"/>
<connect gate="G$1" pin="PC5" pad="21"/>
<connect gate="G$1" pin="PC6" pad="22"/>
<connect gate="G$1" pin="PC7" pad="23"/>
<connect gate="G$1" pin="PD0" pad="26"/>
<connect gate="G$1" pin="PD1" pad="27"/>
<connect gate="G$1" pin="PD2" pad="28"/>
<connect gate="G$1" pin="PD3" pad="29"/>
<connect gate="G$1" pin="PD4" pad="30"/>
<connect gate="G$1" pin="PD5" pad="31"/>
<connect gate="G$1" pin="PD6" pad="32"/>
<connect gate="G$1" pin="PD7" pad="33"/>
<connect gate="G$1" pin="PDI_DATA" pad="56"/>
<connect gate="G$1" pin="PE0" pad="36"/>
<connect gate="G$1" pin="PE1" pad="37"/>
<connect gate="G$1" pin="PE2" pad="38"/>
<connect gate="G$1" pin="PE3" pad="39"/>
<connect gate="G$1" pin="PE4" pad="40"/>
<connect gate="G$1" pin="PE5" pad="41"/>
<connect gate="G$1" pin="PE6" pad="42"/>
<connect gate="G$1" pin="PE7" pad="43"/>
<connect gate="G$1" pin="PF0" pad="46"/>
<connect gate="G$1" pin="PF1" pad="47"/>
<connect gate="G$1" pin="PF2" pad="48"/>
<connect gate="G$1" pin="PF3" pad="49"/>
<connect gate="G$1" pin="PF4" pad="50"/>
<connect gate="G$1" pin="PF5" pad="51"/>
<connect gate="G$1" pin="PF6" pad="54"/>
<connect gate="G$1" pin="PF7" pad="55"/>
<connect gate="G$1" pin="PR0" pad="58"/>
<connect gate="G$1" pin="PR1" pad="59"/>
<connect gate="G$1" pin="RESET" pad="57"/>
<connect gate="G$1" pin="VCC1" pad="15"/>
<connect gate="G$1" pin="VCC2" pad="25"/>
<connect gate="G$1" pin="VCC3" pad="35"/>
<connect gate="G$1" pin="VCC4" pad="45"/>
<connect gate="G$1" pin="VCC5" pad="53"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="CAPACITOR">
<gates>
<gate name="C1" symbol="CAPACITOR" x="0" y="0"/>
</gates>
<devices>
<device name="0402" package="0402">
<connects>
<connect gate="C1" pin="1" pad="1"/>
<connect gate="C1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="0603" package="0603">
<connects>
<connect gate="C1" pin="1" pad="1"/>
<connect gate="C1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="0805" package="0805">
<connects>
<connect gate="C1" pin="1" pad="1"/>
<connect gate="C1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="" package="1206">
<connects>
<connect gate="C1" pin="1" pad="1"/>
<connect gate="C1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="LEG">
<gates>
<gate name="G$1" symbol="LEG" x="0" y="0"/>
</gates>
<devices>
<device name="" package="LEG">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="SUPERCAP">
<gates>
<gate name="G$1" symbol="CAPACITOR_POLERIZED" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SUPERCAP">
<connects>
<connect gate="G$1" pin="+" pad="P"/>
<connect gate="G$1" pin="-" pad="N"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="REGULATOR">
<gates>
<gate name="G$1" symbol="REGULATOR" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SOT-25">
<connects>
<connect gate="G$1" pin="CE" pad="CE"/>
<connect gate="G$1" pin="VIN" pad="VIN"/>
<connect gate="G$1" pin="VOUT" pad="VOUT"/>
<connect gate="G$1" pin="VSS" pad="VSS"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="INDUCTOR">
<gates>
<gate name="G$1" symbol="INDUCTOR" x="0" y="0"/>
</gates>
<devices>
<device name="0603" package="0603">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="0805" package="0805">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="" package="1210">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="REGULATOR_5V">
<gates>
<gate name="G$1" symbol="REGULATOR_5V" x="0" y="-2.54"/>
</gates>
<devices>
<device name="" package="SOT-23-6">
<connects>
<connect gate="G$1" pin="EN" pad="3"/>
<connect gate="G$1" pin="GND" pad="2"/>
<connect gate="G$1" pin="SW" pad="1"/>
<connect gate="G$1" pin="VFB" pad="4"/>
<connect gate="G$1" pin="VIN" pad="6"/>
<connect gate="G$1" pin="VOUT" pad="5"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="RESISTOR">
<gates>
<gate name="G$1" symbol="RESISTOR" x="0" y="0"/>
</gates>
<devices>
<device name="" package="0402">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="IR_PHOTODIODE">
<gates>
<gate name="G$1" symbol="PHOTODIODE" x="0" y="0"/>
</gates>
<devices>
<device name="" package="1208-IR">
<connects>
<connect gate="G$1" pin="A" pad="A"/>
<connect gate="G$1" pin="C" pad="C"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="IR_RECEIVER">
<gates>
<gate name="G$1" symbol="PHOTODIODE" x="0" y="0"/>
<gate name="GND" symbol="SUPPLY" x="-12.7" y="2.54" addlevel="request"/>
</gates>
<devices>
<device name="" package="IR_RECEIVER">
<connects>
<connect gate="G$1" pin="A" pad="2"/>
<connect gate="G$1" pin="C" pad="1"/>
<connect gate="GND" pin="1" pad="3"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="IR_LED">
<gates>
<gate name="G$1" symbol="LED" x="0" y="0"/>
</gates>
<devices>
<device name="" package="1208-IR">
<connects>
<connect gate="G$1" pin="A" pad="A"/>
<connect gate="G$1" pin="C" pad="C"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="7408_X4">
<gates>
<gate name="A" symbol="AND_GATE" x="2.54" y="10.16" swaplevel="1"/>
<gate name="B" symbol="AND_GATE" x="2.54" y="27.94" swaplevel="1"/>
<gate name="C" symbol="AND_GATE" x="2.54" y="-7.62" swaplevel="1"/>
<gate name="D" symbol="AND_GATE" x="2.54" y="-25.4" swaplevel="1"/>
<gate name="VCC" symbol="SUPPLY" x="-33.02" y="22.86" addlevel="request"/>
<gate name="GND" symbol="SUPPLY" x="-33.02" y="15.24" addlevel="request"/>
</gates>
<devices>
<device name="S" package="VFQFN-14">
<connects>
<connect gate="A" pin="I0" pad="1"/>
<connect gate="A" pin="I1" pad="2"/>
<connect gate="A" pin="O" pad="3"/>
<connect gate="B" pin="I0" pad="4"/>
<connect gate="B" pin="I1" pad="5"/>
<connect gate="B" pin="O" pad="6"/>
<connect gate="C" pin="I0" pad="9"/>
<connect gate="C" pin="I1" pad="10"/>
<connect gate="C" pin="O" pad="8"/>
<connect gate="D" pin="I0" pad="12"/>
<connect gate="D" pin="I1" pad="13"/>
<connect gate="D" pin="O" pad="11"/>
<connect gate="GND" pin="1" pad="7"/>
<connect gate="VCC" pin="1" pad="14"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="&quot;" package="DHVQFN14-L">
<connects>
<connect gate="A" pin="I0" pad="1"/>
<connect gate="A" pin="I1" pad="2"/>
<connect gate="A" pin="O" pad="3"/>
<connect gate="B" pin="I0" pad="4"/>
<connect gate="B" pin="I1" pad="5"/>
<connect gate="B" pin="O" pad="6"/>
<connect gate="C" pin="I0" pad="9"/>
<connect gate="C" pin="I1" pad="10"/>
<connect gate="C" pin="O" pad="8"/>
<connect gate="D" pin="I0" pad="12"/>
<connect gate="D" pin="I1" pad="13"/>
<connect gate="D" pin="O" pad="11"/>
<connect gate="GND" pin="1" pad="7"/>
<connect gate="VCC" pin="1" pad="14"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="VAR_RESISTOR_X4">
<gates>
<gate name="P0" symbol="VAR_RESISTOR" x="0" y="22.86" swaplevel="1"/>
<gate name="P1" symbol="VAR_RESISTOR" x="0" y="10.16" swaplevel="1"/>
<gate name="P2" symbol="VAR_RESISTOR" x="0" y="-2.54" swaplevel="1"/>
<gate name="P3" symbol="VAR_RESISTOR" x="0" y="-15.24" swaplevel="1"/>
<gate name="VCC" symbol="SUPPLY" x="-33.02" y="22.86" addlevel="request"/>
<gate name="GND" symbol="SUPPLY" x="-33.02" y="17.78" addlevel="request"/>
<gate name="A0" symbol="SUPPLY" x="-33.02" y="10.16" addlevel="request"/>
<gate name="A1" symbol="SUPPLY" x="-33.02" y="5.08" addlevel="request"/>
<gate name="SDA" symbol="SUPPLY" x="-33.02" y="-2.54" addlevel="request"/>
<gate name="SCL" symbol="SUPPLY" x="-33.02" y="-7.62" addlevel="request"/>
<gate name="!RST" symbol="SUPPLY" x="-33.02" y="-15.24" addlevel="request"/>
</gates>
<devices>
<device name="" package="QFN-20">
<connects>
<connect gate="!RST" pin="1" pad="13"/>
<connect gate="A0" pin="1" pad="2"/>
<connect gate="A1" pin="1" pad="14"/>
<connect gate="GND" pin="1" pad="5"/>
<connect gate="P0" pin="1" pad="9"/>
<connect gate="P0" pin="2" pad="10"/>
<connect gate="P1" pin="1" pad="8"/>
<connect gate="P1" pin="2" pad="7"/>
<connect gate="P2" pin="1" pad="18"/>
<connect gate="P2" pin="2" pad="17"/>
<connect gate="P3" pin="1" pad="19"/>
<connect gate="P3" pin="2" pad="20"/>
<connect gate="SCL" pin="1" pad="3"/>
<connect gate="SDA" pin="1" pad="4"/>
<connect gate="VCC" pin="1" pad="15"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="RGB_LED">
<gates>
<gate name="G$1" symbol="RGB_LED" x="0" y="0"/>
</gates>
<devices>
<device name="" package="RGB_LED">
<connects>
<connect gate="G$1" pin="ANODE" pad="2"/>
<connect gate="G$1" pin="B" pad="3"/>
<connect gate="G$1" pin="G" pad="4"/>
<connect gate="G$1" pin="R" pad="1"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="MOTOR">
<gates>
<gate name="G$1" symbol="MOTOR" x="0" y="0"/>
</gates>
<devices>
<device name="" package="MOTOR_PADS">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="RGB_PHOTODIODE">
<gates>
<gate name="G$1" symbol="PHOTODIODE" x="0" y="0"/>
</gates>
<devices>
<device name="" package="RGB_SENSOR">
<connects>
<connect gate="G$1" pin="A" pad="A1 A2"/>
<connect gate="G$1" pin="C" pad="C1 C2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="HALF-MOTOR_CONTROLLER">
<gates>
<gate name="G$1" symbol="HALF-MOTOR" x="0" y="12.7" swaplevel="1"/>
<gate name="G$2" symbol="HALF-MOTOR" x="0" y="-12.7" swaplevel="1"/>
<gate name="VCC" symbol="SUPPLY" x="-40.64" y="12.7" addlevel="request"/>
<gate name="GND" symbol="SUPPLY" x="-40.64" y="0" addlevel="request"/>
</gates>
<devices>
<device name="" package="DFN-10-EJ">
<connects>
<connect gate="G$1" pin="IN1" pad="1"/>
<connect gate="G$1" pin="IN2" pad="2"/>
<connect gate="G$1" pin="OUT1" pad="10"/>
<connect gate="G$1" pin="OUT2" pad="9"/>
<connect gate="G$2" pin="IN1" pad="4"/>
<connect gate="G$2" pin="IN2" pad="5"/>
<connect gate="G$2" pin="OUT1" pad="7"/>
<connect gate="G$2" pin="OUT2" pad="6"/>
<connect gate="GND" pin="1" pad="3"/>
<connect gate="VCC" pin="1" pad="8"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="DIODE">
<gates>
<gate name="G$1" symbol="DIODE" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SOD-123">
<connects>
<connect gate="G$1" pin="A" pad="A"/>
<connect gate="G$1" pin="C" pad="C"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="MRAM">
<gates>
<gate name="G$1" symbol="MRAM" x="0" y="0"/>
</gates>
<devices>
<device name="" package="DFN-8">
<connects>
<connect gate="G$1" pin="!CS" pad="1"/>
<connect gate="G$1" pin="!HOLD" pad="7"/>
<connect gate="G$1" pin="!WP" pad="3"/>
<connect gate="G$1" pin="SCK" pad="6"/>
<connect gate="G$1" pin="SI" pad="5"/>
<connect gate="G$1" pin="SO" pad="2"/>
<connect gate="G$1" pin="VDD" pad="8"/>
<connect gate="G$1" pin="VSS" pad="4"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="MOSFET-N_X2">
<gates>
<gate name="G$1" symbol="N-CHANNEL_MOSFET" x="-2.54" y="10.16"/>
<gate name="G$2" symbol="N-CHANNEL_MOSFET" x="-2.54" y="-10.16"/>
</gates>
<devices>
<device name="" package="SOT-363">
<connects>
<connect gate="G$1" pin="D" pad="6"/>
<connect gate="G$1" pin="G" pad="2"/>
<connect gate="G$1" pin="S" pad="1"/>
<connect gate="G$2" pin="D" pad="3"/>
<connect gate="G$2" pin="G" pad="5"/>
<connect gate="G$2" pin="S" pad="4"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="EDGE_PORT_3">
<gates>
<gate name="G$1" symbol="EDGE_PORT_3" x="0" y="0"/>
</gates>
<devices>
<device name="" package="EDGE_CONNECTOR3">
<connects>
<connect gate="G$1" pin="1" pad="1A 1B"/>
<connect gate="G$1" pin="2" pad="2A 2B"/>
<connect gate="G$1" pin="3" pad="3A 3B"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
</libraries>
<attributes>
</attributes>
<variantdefs>
</variantdefs>
<classes>
<class number="0" name="default" width="0" drill="0">
</class>
</classes>
<parts>
<part name="UC" library="Droplet" deviceset="XMEGA_A3" device="L"/>
<part name="C1" library="Droplet" deviceset="CAPACITOR" device="0402" value=".1uF"/>
<part name="C2" library="Droplet" deviceset="CAPACITOR" device="0402" value=".1uF"/>
<part name="C3" library="Droplet" deviceset="CAPACITOR" device="0402" value=".1uF"/>
<part name="C4" library="Droplet" deviceset="CAPACITOR" device="0402" value=".1uF"/>
<part name="C5" library="Droplet" deviceset="CAPACITOR" device="0402" value=".1uF"/>
<part name="C6" library="Droplet" deviceset="CAPACITOR" device="0402" value=".1uF"/>
<part name="LEG1" library="Droplet" deviceset="LEG" device=""/>
<part name="LEG2" library="Droplet" deviceset="LEG" device=""/>
<part name="LEG3" library="Droplet" deviceset="LEG" device=""/>
<part name="C" library="Droplet" deviceset="SUPERCAP" device="" value="1F"/>
<part name="V1" library="Droplet" deviceset="REGULATOR" device="" value="2.8V"/>
<part name="C7" library="Droplet" deviceset="CAPACITOR" device="0402" value="1uF"/>
<part name="C8" library="Droplet" deviceset="CAPACITOR" device="0402" value="1uF"/>
<part name="C9" library="Droplet" deviceset="CAPACITOR" device="0603" value="10uF"/>
<part name="L1" library="Droplet" deviceset="INDUCTOR" device="0805" value="10uH"/>
<part name="L2" library="Droplet" deviceset="INDUCTOR" device="0603" value=""/>
<part name="C10" library="Droplet" deviceset="CAPACITOR" device="0603" value="10uF"/>
<part name="V2" library="Droplet" deviceset="REGULATOR_5V" device="" value="5V"/>
<part name="L3" library="Droplet" deviceset="INDUCTOR" device="" value="4.7uH"/>
<part name="C11" library="Droplet" deviceset="CAPACITOR" device="0603" value="10uF"/>
<part name="C12" library="Droplet" deviceset="CAPACITOR" device="0603" value="10uF"/>
<part name="R1" library="Droplet" deviceset="RESISTOR" device="" value="845K"/>
<part name="R2" library="Droplet" deviceset="RESISTOR" device="" value="270K"/>
<part name="R3" library="Droplet" deviceset="RESISTOR" device="" value="330"/>
<part name="C13" library="Droplet" deviceset="CAPACITOR" device="0402" value="1uF"/>
<part name="R4" library="Droplet" deviceset="RESISTOR" device="" value="330"/>
<part name="C14" library="Droplet" deviceset="CAPACITOR" device="0402" value="1uF"/>
<part name="R5" library="Droplet" deviceset="RESISTOR" device="" value="330"/>
<part name="C15" library="Droplet" deviceset="CAPACITOR" device="0402" value="1uF"/>
<part name="R6" library="Droplet" deviceset="RESISTOR" device="" value="330"/>
<part name="C16" library="Droplet" deviceset="CAPACITOR" device="0402" value="1uF"/>
<part name="R7" library="Droplet" deviceset="RESISTOR" device="" value="330"/>
<part name="C17" library="Droplet" deviceset="CAPACITOR" device="0402" value="1uF"/>
<part name="R8" library="Droplet" deviceset="RESISTOR" device="" value="330"/>
<part name="C18" library="Droplet" deviceset="CAPACITOR" device="0402" value="1uF"/>
<part name="R9" library="Droplet" deviceset="RESISTOR" device="" value="180K"/>
<part name="S0" library="Droplet" deviceset="IR_PHOTODIODE" device=""/>
<part name="R10" library="Droplet" deviceset="RESISTOR" device="" value="180K"/>
<part name="S1" library="Droplet" deviceset="IR_PHOTODIODE" device=""/>
<part name="R11" library="Droplet" deviceset="RESISTOR" device="" value="180K"/>
<part name="S2" library="Droplet" deviceset="IR_PHOTODIODE" device=""/>
<part name="R12" library="Droplet" deviceset="RESISTOR" device="" value="180K"/>
<part name="S3" library="Droplet" deviceset="IR_PHOTODIODE" device=""/>
<part name="R13" library="Droplet" deviceset="RESISTOR" device="" value="180K"/>
<part name="S4" library="Droplet" deviceset="IR_PHOTODIODE" device=""/>
<part name="R14" library="Droplet" deviceset="RESISTOR" device="" value="180K"/>
<part name="S5" library="Droplet" deviceset="IR_PHOTODIODE" device=""/>
<part name="RX0" library="Droplet" deviceset="IR_RECEIVER" device=""/>
<part name="RX1" library="Droplet" deviceset="IR_RECEIVER" device=""/>
<part name="RX2" library="Droplet" deviceset="IR_RECEIVER" device=""/>
<part name="RX3" library="Droplet" deviceset="IR_RECEIVER" device=""/>
<part name="RX4" library="Droplet" deviceset="IR_RECEIVER" device=""/>
<part name="RX5" library="Droplet" deviceset="IR_RECEIVER" device=""/>
<part name="E0" library="Droplet" deviceset="IR_LED" device="" value=""/>
<part name="E1" library="Droplet" deviceset="IR_LED" device="" value=""/>
<part name="E2" library="Droplet" deviceset="IR_LED" device="" value=""/>
<part name="E3" library="Droplet" deviceset="IR_LED" device="" value=""/>
<part name="E4" library="Droplet" deviceset="IR_LED" device="" value=""/>
<part name="E5" library="Droplet" deviceset="IR_LED" device="" value=""/>
<part name="Z1" library="Droplet" deviceset="VAR_RESISTOR_X4" device=""/>
<part name="Z2" library="Droplet" deviceset="VAR_RESISTOR_X4" device=""/>
<part name="LED" library="Droplet" deviceset="RGB_LED" device=""/>
<part name="MOT1" library="Droplet" deviceset="MOTOR" device=""/>
<part name="MOT2" library="Droplet" deviceset="MOTOR" device=""/>
<part name="MOT3" library="Droplet" deviceset="MOTOR" device=""/>
<part name="PD_R" library="Droplet" deviceset="RGB_PHOTODIODE" device=""/>
<part name="PD_G" library="Droplet" deviceset="RGB_PHOTODIODE" device=""/>
<part name="PD_B" library="Droplet" deviceset="RGB_PHOTODIODE" device=""/>
<part name="R18" library="Droplet" deviceset="RESISTOR" device="" value="180K"/>
<part name="R19" library="Droplet" deviceset="RESISTOR" device="" value="180K"/>
<part name="R20" library="Droplet" deviceset="RESISTOR" device="" value="180K"/>
<part name="M1" library="Droplet" deviceset="HALF-MOTOR_CONTROLLER" device=""/>
<part name="M2" library="Droplet" deviceset="HALF-MOTOR_CONTROLLER" device=""/>
<part name="R21" library="Droplet" deviceset="RESISTOR" device="" value="10K"/>
<part name="R22" library="Droplet" deviceset="RESISTOR" device="" value="10K"/>
<part name="R23" library="Droplet" deviceset="RESISTOR" device="" value="10K"/>
<part name="R24" library="Droplet" deviceset="RESISTOR" device="" value="10K"/>
<part name="R25" library="Droplet" deviceset="RESISTOR" device="" value="43K"/>
<part name="R26" library="Droplet" deviceset="RESISTOR" device="" value="47K"/>
<part name="R27" library="Droplet" deviceset="RESISTOR" device="" value="10K"/>
<part name="R28" library="Droplet" deviceset="RESISTOR" device="" value="43K"/>
<part name="R29" library="Droplet" deviceset="RESISTOR" device="" value="47K"/>
<part name="R30" library="Droplet" deviceset="RESISTOR" device="" value="10K"/>
<part name="R31" library="Droplet" deviceset="RESISTOR" device="" value="43K"/>
<part name="R32" library="Droplet" deviceset="RESISTOR" device="" value="47K"/>
<part name="R33" library="Droplet" deviceset="RESISTOR" device="" value="43K"/>
<part name="R34" library="Droplet" deviceset="RESISTOR" device="" value="180K"/>
<part name="A1" library="Droplet" deviceset="7408_X4" device="&quot;">
<attribute name="DIGIKEY" value="568-2993-1-ND"/>
</part>
<part name="A2" library="Droplet" deviceset="7408_X4" device="&quot;"/>
<part name="R35" library="Droplet" deviceset="RESISTOR" device="" value="200K"/>
<part name="R36" library="Droplet" deviceset="RESISTOR" device="" value="33K"/>
<part name="R37" library="Droplet" deviceset="RESISTOR" device="" value="180K"/>
<part name="R38" library="Droplet" deviceset="RESISTOR" device="" value="10K"/>
<part name="C22" library="Droplet" deviceset="CAPACITOR" device="0603" value="10uF"/>
<part name="C23" library="Droplet" deviceset="CAPACITOR" device="0603" value="10uF"/>
<part name="R39" library="Droplet" deviceset="RESISTOR" device="" value="180K"/>
<part name="R40" library="Droplet" deviceset="RESISTOR" device="" value="22K"/>
<part name="D1" library="Droplet" deviceset="DIODE" device=""/>
<part name="D5" library="Droplet" deviceset="DIODE" device=""/>
<part name="D3" library="Droplet" deviceset="DIODE" device=""/>
<part name="D4" library="Droplet" deviceset="DIODE" device=""/>
<part name="D2" library="Droplet" deviceset="DIODE" device=""/>
<part name="D6" library="Droplet" deviceset="DIODE" device=""/>
<part name="C25" library="Droplet" deviceset="CAPACITOR" device="0805" value="22uF"/>
<part name="C26" library="Droplet" deviceset="CAPACITOR" device="0805" value="22uF"/>
<part name="C27" library="Droplet" deviceset="CAPACITOR" device="0805" value="22uF"/>
<part name="C24" library="Droplet" deviceset="CAPACITOR" device="0402" value=".1uF"/>
<part name="C28" library="Droplet" deviceset="CAPACITOR" device="0402" value=".1uF"/>
<part name="C29" library="Droplet" deviceset="CAPACITOR" device="0402" value=".1uF"/>
<part name="C30" library="Droplet" deviceset="CAPACITOR" device="0402" value=".1uF"/>
<part name="C31" library="Droplet" deviceset="CAPACITOR" device="0402" value=".1uF"/>
<part name="C32" library="Droplet" deviceset="CAPACITOR" device="0402" value=".1uF"/>
<part name="C33" library="Droplet" deviceset="CAPACITOR" device="0402" value=".1uF"/>
<part name="MEM" library="Droplet" deviceset="MRAM" device=""/>
<part name="F1" library="Droplet" deviceset="MOSFET-N_X2" device=""/>
<part name="F2" library="Droplet" deviceset="MOSFET-N_X2" device=""/>
<part name="R15" library="Droplet" deviceset="RESISTOR" device="" value="75"/>
<part name="R16" library="Droplet" deviceset="RESISTOR" device="" value="75"/>
<part name="R17" library="Droplet" deviceset="RESISTOR" device="" value="75"/>
<part name="R41" library="Droplet" deviceset="RESISTOR" device="" value="27"/>
<part name="R42" library="Droplet" deviceset="RESISTOR" device="" value="27"/>
<part name="R43" library="Droplet" deviceset="RESISTOR" device="" value="27"/>
<part name="P1" library="Droplet" deviceset="EDGE_PORT_3" device=""/>
<part name="P2" library="Droplet" deviceset="EDGE_PORT_3" device=""/>
<part name="P3" library="Droplet" deviceset="EDGE_PORT_3" device=""/>
</parts>
<sheets>
<sheet>
<plain>
<wire x1="116.84" y1="304.8" x2="116.84" y2="157.48" width="0.1524" layer="98"/>
<wire x1="116.84" y1="157.48" x2="226.06" y2="157.48" width="0.1524" layer="98"/>
<wire x1="226.06" y1="157.48" x2="226.06" y2="304.8" width="0.1524" layer="98"/>
<wire x1="226.06" y1="304.8" x2="116.84" y2="304.8" width="0.1524" layer="98"/>
<text x="116.84" y="307.34" size="2.54" layer="98">Power Regulation and Filtering</text>
<wire x1="2.54" y1="5.08" x2="2.54" y2="144.78" width="0.1524" layer="98"/>
<wire x1="2.54" y1="144.78" x2="109.22" y2="144.78" width="0.1524" layer="98"/>
<wire x1="109.22" y1="144.78" x2="109.22" y2="5.08" width="0.1524" layer="98"/>
<wire x1="109.22" y1="5.08" x2="2.54" y2="5.08" width="0.1524" layer="98"/>
<wire x1="0" y1="147.32" x2="0" y2="40.64" width="0.1524" layer="98"/>
<wire x1="0" y1="40.64" x2="154.94" y2="40.64" width="0.1524" layer="98"/>
<wire x1="154.94" y1="40.64" x2="154.94" y2="147.32" width="0.1524" layer="98"/>
<wire x1="154.94" y1="147.32" x2="0" y2="147.32" width="0.1524" layer="98"/>
<text x="0" y="149.86" size="2.54" layer="98">Infrared Communication</text>
<text x="2.54" y="0" size="2.54" layer="98">Range &amp; Bearing</text>
<wire x1="160.02" y1="86.36" x2="160.02" y2="0" width="0.1524" layer="98"/>
<wire x1="160.02" y1="0" x2="236.22" y2="0" width="0.1524" layer="98"/>
<wire x1="236.22" y1="0" x2="236.22" y2="86.36" width="0.1524" layer="98"/>
<wire x1="236.22" y1="86.36" x2="160.02" y2="86.36" width="0.1524" layer="98"/>
<text x="160.02" y="88.9" size="2.54" layer="98">RGB Emitter and Sensors</text>
<wire x1="160.02" y1="149.86" x2="160.02" y2="93.98" width="0.1524" layer="98"/>
<wire x1="226.06" y1="149.86" x2="160.02" y2="149.86" width="0.1524" layer="98"/>
<text x="160.02" y="152.4" size="2.54" layer="98">Motor Controllers</text>
<wire x1="114.3" y1="30.48" x2="114.3" y2="0" width="0.1524" layer="98"/>
<wire x1="114.3" y1="0" x2="154.94" y2="0" width="0.1524" layer="98"/>
<wire x1="154.94" y1="0" x2="154.94" y2="30.48" width="0.1524" layer="98"/>
<wire x1="154.94" y1="30.48" x2="114.3" y2="30.48" width="0.1524" layer="98"/>
<text x="114.3" y="33.02" size="2.54" layer="98">Pullups</text>
<wire x1="226.06" y1="149.86" x2="226.06" y2="93.98" width="0.1524" layer="98"/>
<wire x1="226.06" y1="93.98" x2="160.02" y2="93.98" width="0.1524" layer="98"/>
<wire x1="0" y1="157.48" x2="109.22" y2="157.48" width="0.1524" layer="98"/>
<wire x1="109.22" y1="157.48" x2="109.22" y2="304.8" width="0.1524" layer="98"/>
<wire x1="109.22" y1="304.8" x2="0" y2="304.8" width="0.1524" layer="98"/>
<text x="0" y="307.34" size="2.54" layer="98">Microcontroller and External Connections</text>
<wire x1="0" y1="304.8" x2="0" y2="157.48" width="0.1524" layer="98"/>
<wire x1="231.14" y1="101.6" x2="231.14" y2="304.8" width="0.1524" layer="98"/>
<wire x1="231.14" y1="304.8" x2="266.7" y2="304.8" width="0.1524" layer="98"/>
<text x="231.14" y="307.34" size="2.54" layer="98">Power Monitoring</text>
<wire x1="266.7" y1="0" x2="266.7" y2="304.8" width="0.1524" layer="98"/>
<wire x1="266.7" y1="0" x2="241.3" y2="0" width="0.1524" layer="98"/>
<wire x1="241.3" y1="0" x2="241.3" y2="101.6" width="0.1524" layer="98"/>
<wire x1="241.3" y1="101.6" x2="231.14" y2="101.6" width="0.1524" layer="98"/>
<text x="271.78" y="307.34" size="2.54" layer="98">Optional Memory</text>
<wire x1="322.58" y1="304.8" x2="271.78" y2="304.8" width="0.1524" layer="98"/>
<wire x1="322.58" y1="281.94" x2="271.78" y2="281.94" width="0.1524" layer="98"/>
<wire x1="271.78" y1="281.94" x2="271.78" y2="304.8" width="0.1524" layer="98"/>
<wire x1="322.58" y1="281.94" x2="322.58" y2="304.8" width="0.1524" layer="98"/>
</plain>
<instances>
<instance part="UC" gate="G$1" x="55.88" y="231.14"/>
<instance part="C1" gate="C1" x="139.7" y="238.76"/>
<instance part="C2" gate="C1" x="147.32" y="238.76"/>
<instance part="C3" gate="C1" x="154.94" y="238.76"/>
<instance part="C4" gate="C1" x="162.56" y="238.76"/>
<instance part="C5" gate="C1" x="170.18" y="238.76"/>
<instance part="C6" gate="C1" x="182.88" y="238.76"/>
<instance part="LEG1" gate="G$1" x="127" y="175.26"/>
<instance part="LEG2" gate="G$1" x="160.02" y="175.26"/>
<instance part="LEG3" gate="G$1" x="193.04" y="175.26"/>
<instance part="C" gate="G$1" x="127" y="241.3"/>
<instance part="V1" gate="G$1" x="149.86" y="289.56"/>
<instance part="C7" gate="C1" x="129.54" y="287.02"/>
<instance part="C8" gate="C1" x="162.56" y="287.02"/>
<instance part="C9" gate="C1" x="182.88" y="287.02"/>
<instance part="L1" gate="G$1" x="172.72" y="294.64"/>
<instance part="L2" gate="G$1" x="193.04" y="294.64"/>
<instance part="C10" gate="C1" x="203.2" y="287.02"/>
<instance part="V2" gate="G$1" x="170.18" y="264.16"/>
<instance part="L3" gate="G$1" x="149.86" y="261.62"/>
<instance part="C11" gate="C1" x="129.54" y="261.62"/>
<instance part="C12" gate="C1" x="198.12" y="261.62"/>
<instance part="R1" gate="G$1" x="182.88" y="264.16" rot="R90"/>
<instance part="R2" gate="G$1" x="190.5" y="256.54"/>
<instance part="R3" gate="G$1" x="132.08" y="218.44" rot="R90"/>
<instance part="C13" gate="C1" x="132.08" y="203.2"/>
<instance part="R4" gate="G$1" x="147.32" y="218.44" rot="R90"/>
<instance part="C14" gate="C1" x="147.32" y="203.2"/>
<instance part="R5" gate="G$1" x="162.56" y="218.44" rot="R90"/>
<instance part="C15" gate="C1" x="162.56" y="203.2"/>
<instance part="R6" gate="G$1" x="177.8" y="218.44" rot="R90"/>
<instance part="C16" gate="C1" x="177.8" y="203.2"/>
<instance part="R7" gate="G$1" x="193.04" y="218.44" rot="R90"/>
<instance part="C17" gate="C1" x="193.04" y="203.2"/>
<instance part="R8" gate="G$1" x="208.28" y="218.44" rot="R90"/>
<instance part="C18" gate="C1" x="208.28" y="203.2"/>
<instance part="R9" gate="G$1" x="15.24" y="17.78" rot="R90"/>
<instance part="S0" gate="G$1" x="15.24" y="30.48"/>
<instance part="R10" gate="G$1" x="30.48" y="17.78" rot="R90"/>
<instance part="S1" gate="G$1" x="30.48" y="30.48"/>
<instance part="R11" gate="G$1" x="45.72" y="17.78" rot="R90"/>
<instance part="S2" gate="G$1" x="45.72" y="30.48"/>
<instance part="R12" gate="G$1" x="60.96" y="17.78" rot="R90"/>
<instance part="S3" gate="G$1" x="60.96" y="30.48"/>
<instance part="R13" gate="G$1" x="76.2" y="17.78" rot="R90"/>
<instance part="S4" gate="G$1" x="76.2" y="30.48"/>
<instance part="R14" gate="G$1" x="91.44" y="17.78" rot="R90"/>
<instance part="S5" gate="G$1" x="91.44" y="30.48"/>
<instance part="RX0" gate="G$1" x="129.54" y="137.16" rot="R270"/>
<instance part="RX1" gate="G$1" x="129.54" y="121.92" rot="R270"/>
<instance part="RX2" gate="G$1" x="129.54" y="106.68" rot="R270"/>
<instance part="RX3" gate="G$1" x="129.54" y="91.44" rot="R270"/>
<instance part="RX4" gate="G$1" x="129.54" y="76.2" rot="R270"/>
<instance part="RX5" gate="G$1" x="129.54" y="60.96" rot="R270"/>
<instance part="E0" gate="G$1" x="58.42" y="132.08" rot="R90"/>
<instance part="E1" gate="G$1" x="58.42" y="116.84" rot="R90"/>
<instance part="E2" gate="G$1" x="58.42" y="101.6" rot="R90"/>
<instance part="E3" gate="G$1" x="58.42" y="81.28" rot="R90"/>
<instance part="E4" gate="G$1" x="58.42" y="66.04" rot="R90"/>
<instance part="E5" gate="G$1" x="58.42" y="50.8" rot="R90"/>
<instance part="Z1" gate="P0" x="48.26" y="132.08"/>
<instance part="Z1" gate="P1" x="48.26" y="116.84"/>
<instance part="Z1" gate="P2" x="48.26" y="101.6"/>
<instance part="Z2" gate="P2" x="48.26" y="81.28"/>
<instance part="Z2" gate="P0" x="48.26" y="66.04"/>
<instance part="Z2" gate="P1" x="48.26" y="50.8"/>
<instance part="LED" gate="G$1" x="203.2" y="43.18" rot="R270"/>
<instance part="MOT1" gate="G$1" x="200.66" y="139.7"/>
<instance part="MOT2" gate="G$1" x="200.66" y="121.92"/>
<instance part="MOT3" gate="G$1" x="200.66" y="104.14" rot="R180"/>
<instance part="PD_R" gate="G$1" x="177.8" y="78.74"/>
<instance part="PD_G" gate="G$1" x="198.12" y="78.74"/>
<instance part="PD_B" gate="G$1" x="218.44" y="78.74"/>
<instance part="R18" gate="G$1" x="177.8" y="66.04" rot="R90"/>
<instance part="R19" gate="G$1" x="198.12" y="66.04" rot="R90"/>
<instance part="R20" gate="G$1" x="218.44" y="66.04" rot="R90"/>
<instance part="M1" gate="G$2" x="177.8" y="139.7"/>
<instance part="M1" gate="G$1" x="177.8" y="121.92"/>
<instance part="M2" gate="G$2" x="177.8" y="104.14"/>
<instance part="Z1" gate="A0" x="96.52" y="111.76"/>
<instance part="Z1" gate="A1" x="96.52" y="106.68"/>
<instance part="Z2" gate="A0" x="96.52" y="55.88"/>
<instance part="Z2" gate="A1" x="96.52" y="50.8"/>
<instance part="M1" gate="VCC" x="220.98" y="134.62"/>
<instance part="M2" gate="VCC" x="220.98" y="116.84"/>
<instance part="R21" gate="G$1" x="134.62" y="22.86"/>
<instance part="R22" gate="G$1" x="134.62" y="15.24"/>
<instance part="R23" gate="G$1" x="134.62" y="7.62"/>
<instance part="Z2" gate="!RST" x="96.52" y="45.72"/>
<instance part="Z1" gate="!RST" x="96.52" y="101.6"/>
<instance part="R24" gate="G$1" x="243.84" y="259.08" rot="R90"/>
<instance part="R25" gate="G$1" x="243.84" y="274.32" rot="R90"/>
<instance part="R26" gate="G$1" x="243.84" y="289.56" rot="R90"/>
<instance part="R27" gate="G$1" x="243.84" y="157.48" rot="R90"/>
<instance part="R28" gate="G$1" x="243.84" y="172.72" rot="R90"/>
<instance part="R29" gate="G$1" x="243.84" y="187.96" rot="R90"/>
<instance part="R30" gate="G$1" x="243.84" y="208.28" rot="R90"/>
<instance part="R31" gate="G$1" x="243.84" y="223.52" rot="R90"/>
<instance part="R32" gate="G$1" x="243.84" y="238.76" rot="R90"/>
<instance part="R33" gate="G$1" x="243.84" y="121.92" rot="R90"/>
<instance part="R34" gate="G$1" x="243.84" y="137.16" rot="R90"/>
<instance part="M1" gate="GND" x="220.98" y="129.54"/>
<instance part="M2" gate="GND" x="220.98" y="111.76"/>
<instance part="Z1" gate="GND" x="96.52" y="116.84"/>
<instance part="Z1" gate="VCC" x="96.52" y="121.92"/>
<instance part="Z1" gate="SCL" x="96.52" y="127"/>
<instance part="Z1" gate="SDA" x="96.52" y="132.08"/>
<instance part="Z2" gate="GND" x="96.52" y="60.96"/>
<instance part="Z2" gate="VCC" x="96.52" y="66.04"/>
<instance part="Z2" gate="SCL" x="96.52" y="71.12"/>
<instance part="Z2" gate="SDA" x="96.52" y="76.2"/>
<instance part="RX0" gate="GND" x="147.32" y="129.54"/>
<instance part="RX1" gate="GND" x="147.32" y="114.3"/>
<instance part="RX3" gate="GND" x="147.32" y="83.82"/>
<instance part="RX2" gate="GND" x="147.32" y="99.06"/>
<instance part="RX5" gate="GND" x="147.32" y="53.34"/>
<instance part="RX4" gate="GND" x="147.32" y="68.58"/>
<instance part="A1" gate="A" x="30.48" y="132.08">
<attribute name="DIGIKEY" x="30.48" y="132.08" size="1.778" layer="96" display="off"/>
</instance>
<instance part="A1" gate="B" x="30.48" y="116.84"/>
<instance part="A1" gate="C" x="30.48" y="101.6"/>
<instance part="A2" gate="B" x="30.48" y="81.28"/>
<instance part="A2" gate="C" x="30.48" y="66.04"/>
<instance part="A2" gate="A" x="30.48" y="50.8"/>
<instance part="A1" gate="GND" x="96.52" y="96.52"/>
<instance part="A1" gate="VCC" x="96.52" y="91.44"/>
<instance part="A2" gate="GND" x="96.52" y="86.36"/>
<instance part="A2" gate="VCC" x="96.52" y="81.28"/>
<instance part="R35" gate="G$1" x="251.46" y="101.6" rot="R90"/>
<instance part="R36" gate="G$1" x="251.46" y="86.36" rot="R90"/>
<instance part="R37" gate="G$1" x="251.46" y="66.04" rot="R90"/>
<instance part="R38" gate="G$1" x="251.46" y="50.8" rot="R90"/>
<instance part="C22" gate="C1" x="200.66" y="238.76"/>
<instance part="C23" gate="C1" x="210.82" y="238.76"/>
<instance part="R39" gate="G$1" x="251.46" y="30.48" rot="R90"/>
<instance part="R40" gate="G$1" x="251.46" y="15.24" rot="R90"/>
<instance part="D1" gate="G$1" x="132.08" y="193.04"/>
<instance part="D5" gate="G$1" x="198.12" y="193.04"/>
<instance part="D3" gate="G$1" x="165.1" y="193.04"/>
<instance part="D4" gate="G$1" x="165.1" y="187.96" rot="R180"/>
<instance part="D2" gate="G$1" x="132.08" y="187.96" rot="R180"/>
<instance part="D6" gate="G$1" x="198.12" y="187.96" rot="R180"/>
<instance part="C25" gate="C1" x="142.24" y="185.42"/>
<instance part="C26" gate="C1" x="175.26" y="185.42"/>
<instance part="C27" gate="C1" x="208.28" y="185.42"/>
<instance part="C24" gate="C1" x="259.08" y="83.82"/>
<instance part="C28" gate="C1" x="259.08" y="48.26"/>
<instance part="C29" gate="C1" x="259.08" y="12.7"/>
<instance part="C30" gate="C1" x="251.46" y="119.38"/>
<instance part="C31" gate="C1" x="185.42" y="63.5"/>
<instance part="C32" gate="C1" x="205.74" y="63.5"/>
<instance part="C33" gate="C1" x="226.06" y="63.5"/>
<instance part="MEM" gate="G$1" x="302.26" y="294.64"/>
<instance part="F1" gate="G$1" x="180.34" y="12.7" smashed="yes"/>
<instance part="F1" gate="G$2" x="200.66" y="12.7" smashed="yes"/>
<instance part="F2" gate="G$1" x="220.98" y="12.7" smashed="yes"/>
<instance part="R15" gate="G$1" x="223.52" y="25.4" rot="R90"/>
<instance part="R16" gate="G$1" x="203.2" y="25.4" rot="R90"/>
<instance part="R17" gate="G$1" x="182.88" y="25.4" rot="R90"/>
<instance part="R41" gate="G$1" x="193.04" y="144.78" rot="R180"/>
<instance part="R42" gate="G$1" x="193.04" y="127" rot="R180"/>
<instance part="R43" gate="G$1" x="193.04" y="109.22" rot="R180"/>
<instance part="P1" gate="G$1" x="20.32" y="198.12"/>
<instance part="P2" gate="G$1" x="20.32" y="182.88"/>
<instance part="P3" gate="G$1" x="20.32" y="167.64"/>
</instances>
<busses>
</busses>
<nets>
<net name="VCC" class="0">
<segment>
<pinref part="UC" gate="G$1" pin="VCC1"/>
<wire x1="5.08" y1="248.92" x2="15.24" y2="248.92" width="0.1524" layer="91"/>
<pinref part="UC" gate="G$1" pin="VCC5"/>
<wire x1="15.24" y1="248.92" x2="22.86" y2="248.92" width="0.1524" layer="91"/>
<wire x1="15.24" y1="248.92" x2="15.24" y2="246.38" width="0.1524" layer="91"/>
<wire x1="15.24" y1="246.38" x2="15.24" y2="243.84" width="0.1524" layer="91"/>
<wire x1="15.24" y1="243.84" x2="15.24" y2="241.3" width="0.1524" layer="91"/>
<wire x1="15.24" y1="241.3" x2="15.24" y2="238.76" width="0.1524" layer="91"/>
<wire x1="15.24" y1="238.76" x2="22.86" y2="238.76" width="0.1524" layer="91"/>
<junction x="15.24" y="248.92"/>
<pinref part="UC" gate="G$1" pin="VCC4"/>
<wire x1="22.86" y1="241.3" x2="15.24" y2="241.3" width="0.1524" layer="91"/>
<junction x="15.24" y="241.3"/>
<pinref part="UC" gate="G$1" pin="VCC3"/>
<wire x1="22.86" y1="243.84" x2="15.24" y2="243.84" width="0.1524" layer="91"/>
<junction x="15.24" y="243.84"/>
<pinref part="UC" gate="G$1" pin="VCC2"/>
<wire x1="22.86" y1="246.38" x2="15.24" y2="246.38" width="0.1524" layer="91"/>
<junction x="15.24" y="246.38"/>
<label x="5.08" y="248.92" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="C1" gate="C1" pin="1"/>
<wire x1="139.7" y1="243.84" x2="139.7" y2="246.38" width="0.1524" layer="91"/>
<pinref part="C5" gate="C1" pin="1"/>
<wire x1="139.7" y1="246.38" x2="147.32" y2="246.38" width="0.1524" layer="91"/>
<wire x1="147.32" y1="246.38" x2="154.94" y2="246.38" width="0.1524" layer="91"/>
<wire x1="154.94" y1="246.38" x2="162.56" y2="246.38" width="0.1524" layer="91"/>
<wire x1="162.56" y1="246.38" x2="170.18" y2="246.38" width="0.1524" layer="91"/>
<wire x1="170.18" y1="246.38" x2="170.18" y2="243.84" width="0.1524" layer="91"/>
<pinref part="C4" gate="C1" pin="1"/>
<wire x1="162.56" y1="243.84" x2="162.56" y2="246.38" width="0.1524" layer="91"/>
<junction x="162.56" y="246.38"/>
<pinref part="C3" gate="C1" pin="1"/>
<wire x1="154.94" y1="243.84" x2="154.94" y2="246.38" width="0.1524" layer="91"/>
<junction x="154.94" y="246.38"/>
<pinref part="C2" gate="C1" pin="1"/>
<wire x1="147.32" y1="243.84" x2="147.32" y2="246.38" width="0.1524" layer="91"/>
<junction x="147.32" y="246.38"/>
<label x="134.62" y="246.38" size="1.778" layer="95"/>
<wire x1="139.7" y1="246.38" x2="134.62" y2="246.38" width="0.1524" layer="91"/>
<junction x="139.7" y="246.38"/>
</segment>
<segment>
<pinref part="L1" gate="G$1" pin="2"/>
<pinref part="L2" gate="G$1" pin="1"/>
<wire x1="180.34" y1="294.64" x2="182.88" y2="294.64" width="0.1524" layer="91"/>
<pinref part="C9" gate="C1" pin="1"/>
<wire x1="182.88" y1="294.64" x2="185.42" y2="294.64" width="0.1524" layer="91"/>
<wire x1="182.88" y1="294.64" x2="182.88" y2="292.1" width="0.1524" layer="91"/>
<junction x="182.88" y="294.64"/>
<wire x1="182.88" y1="294.64" x2="182.88" y2="299.72" width="0.1524" layer="91"/>
<wire x1="182.88" y1="299.72" x2="180.34" y2="299.72" width="0.1524" layer="91"/>
<label x="180.34" y="299.72" size="1.778" layer="95"/>
<wire x1="182.88" y1="299.72" x2="185.42" y2="299.72" width="0.1524" layer="91"/>
<junction x="182.88" y="299.72"/>
</segment>
<segment>
<pinref part="R8" gate="G$1" pin="2"/>
<wire x1="208.28" y1="223.52" x2="208.28" y2="226.06" width="0.1524" layer="91"/>
<wire x1="208.28" y1="226.06" x2="193.04" y2="226.06" width="0.1524" layer="91"/>
<pinref part="R3" gate="G$1" pin="2"/>
<wire x1="193.04" y1="226.06" x2="177.8" y2="226.06" width="0.1524" layer="91"/>
<wire x1="177.8" y1="226.06" x2="162.56" y2="226.06" width="0.1524" layer="91"/>
<wire x1="162.56" y1="226.06" x2="147.32" y2="226.06" width="0.1524" layer="91"/>
<wire x1="147.32" y1="226.06" x2="132.08" y2="226.06" width="0.1524" layer="91"/>
<wire x1="132.08" y1="226.06" x2="121.92" y2="226.06" width="0.1524" layer="91"/>
<wire x1="132.08" y1="223.52" x2="132.08" y2="226.06" width="0.1524" layer="91"/>
<junction x="132.08" y="226.06"/>
<pinref part="R4" gate="G$1" pin="2"/>
<wire x1="147.32" y1="223.52" x2="147.32" y2="226.06" width="0.1524" layer="91"/>
<junction x="147.32" y="226.06"/>
<pinref part="R5" gate="G$1" pin="2"/>
<wire x1="162.56" y1="223.52" x2="162.56" y2="226.06" width="0.1524" layer="91"/>
<junction x="162.56" y="226.06"/>
<pinref part="R6" gate="G$1" pin="2"/>
<wire x1="177.8" y1="223.52" x2="177.8" y2="226.06" width="0.1524" layer="91"/>
<junction x="177.8" y="226.06"/>
<pinref part="R7" gate="G$1" pin="2"/>
<wire x1="193.04" y1="223.52" x2="193.04" y2="226.06" width="0.1524" layer="91"/>
<junction x="193.04" y="226.06"/>
<label x="121.92" y="226.06" size="1.778" layer="95"/>
<wire x1="208.28" y1="226.06" x2="218.44" y2="226.06" width="0.1524" layer="91"/>
<junction x="208.28" y="226.06"/>
<label x="215.9" y="226.06" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="Z2" gate="A0" pin="1"/>
<wire x1="91.44" y1="55.88" x2="88.9" y2="55.88" width="0.1524" layer="91"/>
<label x="83.82" y="55.88" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="R21" gate="G$1" pin="1"/>
<wire x1="129.54" y1="22.86" x2="127" y2="22.86" width="0.1524" layer="91"/>
<pinref part="R23" gate="G$1" pin="1"/>
<wire x1="127" y1="22.86" x2="127" y2="15.24" width="0.1524" layer="91"/>
<wire x1="127" y1="15.24" x2="127" y2="7.62" width="0.1524" layer="91"/>
<wire x1="127" y1="7.62" x2="129.54" y2="7.62" width="0.1524" layer="91"/>
<pinref part="R22" gate="G$1" pin="1"/>
<wire x1="129.54" y1="15.24" x2="127" y2="15.24" width="0.1524" layer="91"/>
<junction x="127" y="15.24"/>
<wire x1="127" y1="22.86" x2="119.38" y2="22.86" width="0.1524" layer="91"/>
<junction x="127" y="22.86"/>
<label x="119.38" y="22.86" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="Z1" gate="VCC" pin="1"/>
<wire x1="91.44" y1="121.92" x2="88.9" y2="121.92" width="0.1524" layer="91"/>
<label x="83.82" y="121.92" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="Z2" gate="VCC" pin="1"/>
<wire x1="91.44" y1="66.04" x2="88.9" y2="66.04" width="0.1524" layer="91"/>
<label x="83.82" y="66.04" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="A1" gate="VCC" pin="1"/>
<wire x1="91.44" y1="91.44" x2="88.9" y2="91.44" width="0.1524" layer="91"/>
<label x="83.82" y="91.44" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="A2" gate="VCC" pin="1"/>
<wire x1="91.44" y1="81.28" x2="88.9" y2="81.28" width="0.1524" layer="91"/>
<label x="83.82" y="81.28" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="MEM" gate="G$1" pin="VDD"/>
<wire x1="312.42" y1="297.18" x2="317.5" y2="297.18" width="0.1524" layer="91"/>
<label x="312.42" y="297.18" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="MEM" gate="G$1" pin="!WP"/>
<wire x1="312.42" y1="289.56" x2="317.5" y2="289.56" width="0.1524" layer="91"/>
<label x="312.42" y="289.56" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="MEM" gate="G$1" pin="!HOLD"/>
<wire x1="312.42" y1="292.1" x2="317.5" y2="292.1" width="0.1524" layer="91"/>
<label x="312.42" y="292.1" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="P2" gate="G$1" pin="3"/>
<wire x1="17.78" y1="180.34" x2="7.62" y2="180.34" width="0.1524" layer="91"/>
<label x="7.62" y="180.34" size="1.778" layer="95"/>
</segment>
</net>
<net name="AVCC" class="0">
<segment>
<pinref part="UC" gate="G$1" pin="AVCC"/>
<wire x1="22.86" y1="261.62" x2="5.08" y2="261.62" width="0.1524" layer="91"/>
<label x="5.08" y="261.62" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="C6" gate="C1" pin="1"/>
<wire x1="182.88" y1="243.84" x2="182.88" y2="246.38" width="0.1524" layer="91"/>
<wire x1="182.88" y1="246.38" x2="177.8" y2="246.38" width="0.1524" layer="91"/>
<label x="177.8" y="246.38" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="L2" gate="G$1" pin="2"/>
<pinref part="C10" gate="C1" pin="1"/>
<wire x1="200.66" y1="294.64" x2="203.2" y2="294.64" width="0.1524" layer="91"/>
<wire x1="203.2" y1="294.64" x2="203.2" y2="292.1" width="0.1524" layer="91"/>
<label x="208.28" y="294.64" size="1.778" layer="95"/>
<wire x1="203.2" y1="294.64" x2="213.36" y2="294.64" width="0.1524" layer="91"/>
<junction x="203.2" y="294.64"/>
</segment>
<segment>
<pinref part="S5" gate="G$1" pin="C"/>
<wire x1="91.44" y1="33.02" x2="91.44" y2="35.56" width="0.1524" layer="91"/>
<wire x1="91.44" y1="35.56" x2="76.2" y2="35.56" width="0.1524" layer="91"/>
<pinref part="S0" gate="G$1" pin="C"/>
<wire x1="76.2" y1="35.56" x2="60.96" y2="35.56" width="0.1524" layer="91"/>
<wire x1="60.96" y1="35.56" x2="45.72" y2="35.56" width="0.1524" layer="91"/>
<wire x1="45.72" y1="35.56" x2="30.48" y2="35.56" width="0.1524" layer="91"/>
<wire x1="30.48" y1="35.56" x2="15.24" y2="35.56" width="0.1524" layer="91"/>
<wire x1="15.24" y1="35.56" x2="15.24" y2="33.02" width="0.1524" layer="91"/>
<pinref part="S1" gate="G$1" pin="C"/>
<wire x1="30.48" y1="35.56" x2="30.48" y2="33.02" width="0.1524" layer="91"/>
<junction x="30.48" y="35.56"/>
<pinref part="S2" gate="G$1" pin="C"/>
<wire x1="45.72" y1="35.56" x2="45.72" y2="33.02" width="0.1524" layer="91"/>
<junction x="45.72" y="35.56"/>
<pinref part="S3" gate="G$1" pin="C"/>
<wire x1="60.96" y1="35.56" x2="60.96" y2="33.02" width="0.1524" layer="91"/>
<junction x="60.96" y="35.56"/>
<pinref part="S4" gate="G$1" pin="C"/>
<wire x1="76.2" y1="35.56" x2="76.2" y2="33.02" width="0.1524" layer="91"/>
<junction x="76.2" y="35.56"/>
<wire x1="91.44" y1="35.56" x2="101.6" y2="35.56" width="0.1524" layer="91"/>
<junction x="91.44" y="35.56"/>
<wire x1="15.24" y1="35.56" x2="5.08" y2="35.56" width="0.1524" layer="91"/>
<junction x="15.24" y="35.56"/>
<label x="5.08" y="35.56" size="1.778" layer="95"/>
<label x="96.52" y="35.56" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="PD_R" gate="G$1" pin="C"/>
<wire x1="177.8" y1="81.28" x2="177.8" y2="83.82" width="0.1524" layer="91"/>
<wire x1="177.8" y1="83.82" x2="198.12" y2="83.82" width="0.1524" layer="91"/>
<pinref part="PD_B" gate="G$1" pin="C"/>
<wire x1="198.12" y1="83.82" x2="218.44" y2="83.82" width="0.1524" layer="91"/>
<wire x1="218.44" y1="83.82" x2="218.44" y2="81.28" width="0.1524" layer="91"/>
<pinref part="PD_G" gate="G$1" pin="C"/>
<wire x1="198.12" y1="81.28" x2="198.12" y2="83.82" width="0.1524" layer="91"/>
<junction x="198.12" y="83.82"/>
<wire x1="177.8" y1="83.82" x2="167.64" y2="83.82" width="0.1524" layer="91"/>
<junction x="177.8" y="83.82"/>
<label x="167.64" y="83.82" size="1.778" layer="95"/>
</segment>
</net>
<net name="GND" class="0">
<segment>
<pinref part="UC" gate="G$1" pin="GND1"/>
<wire x1="22.86" y1="281.94" x2="15.24" y2="281.94" width="0.1524" layer="91"/>
<pinref part="UC" gate="G$1" pin="GND5"/>
<wire x1="15.24" y1="281.94" x2="5.08" y2="281.94" width="0.1524" layer="91"/>
<wire x1="22.86" y1="271.78" x2="15.24" y2="271.78" width="0.1524" layer="91"/>
<wire x1="15.24" y1="271.78" x2="15.24" y2="274.32" width="0.1524" layer="91"/>
<junction x="15.24" y="281.94"/>
<pinref part="UC" gate="G$1" pin="GND2"/>
<wire x1="15.24" y1="274.32" x2="15.24" y2="276.86" width="0.1524" layer="91"/>
<wire x1="15.24" y1="276.86" x2="15.24" y2="279.4" width="0.1524" layer="91"/>
<wire x1="15.24" y1="279.4" x2="15.24" y2="281.94" width="0.1524" layer="91"/>
<wire x1="22.86" y1="279.4" x2="15.24" y2="279.4" width="0.1524" layer="91"/>
<junction x="15.24" y="279.4"/>
<pinref part="UC" gate="G$1" pin="GND3"/>
<wire x1="22.86" y1="276.86" x2="15.24" y2="276.86" width="0.1524" layer="91"/>
<junction x="15.24" y="276.86"/>
<pinref part="UC" gate="G$1" pin="GND4"/>
<wire x1="22.86" y1="274.32" x2="15.24" y2="274.32" width="0.1524" layer="91"/>
<junction x="15.24" y="274.32"/>
<label x="5.08" y="281.94" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="AGND"/>
<wire x1="22.86" y1="256.54" x2="5.08" y2="256.54" width="0.1524" layer="91"/>
<label x="5.08" y="256.54" size="1.778" layer="95"/>
</segment>
<segment>
<wire x1="134.62" y1="187.96" x2="137.16" y2="187.96" width="0.1524" layer="91"/>
<wire x1="137.16" y1="187.96" x2="137.16" y2="180.34" width="0.1524" layer="91"/>
<wire x1="137.16" y1="180.34" x2="142.24" y2="180.34" width="0.1524" layer="91"/>
<label x="147.32" y="180.34" size="1.778" layer="95"/>
<pinref part="D2" gate="G$1" pin="A"/>
<pinref part="C25" gate="C1" pin="2"/>
<wire x1="142.24" y1="180.34" x2="149.86" y2="180.34" width="0.1524" layer="91"/>
<wire x1="142.24" y1="182.88" x2="142.24" y2="180.34" width="0.1524" layer="91"/>
<junction x="142.24" y="180.34"/>
</segment>
<segment>
<wire x1="167.64" y1="187.96" x2="170.18" y2="187.96" width="0.1524" layer="91"/>
<wire x1="170.18" y1="187.96" x2="170.18" y2="180.34" width="0.1524" layer="91"/>
<wire x1="170.18" y1="180.34" x2="175.26" y2="180.34" width="0.1524" layer="91"/>
<label x="180.34" y="180.34" size="1.778" layer="95"/>
<pinref part="D4" gate="G$1" pin="A"/>
<pinref part="C26" gate="C1" pin="2"/>
<wire x1="175.26" y1="180.34" x2="182.88" y2="180.34" width="0.1524" layer="91"/>
<wire x1="175.26" y1="182.88" x2="175.26" y2="180.34" width="0.1524" layer="91"/>
<junction x="175.26" y="180.34"/>
</segment>
<segment>
<wire x1="200.66" y1="187.96" x2="203.2" y2="187.96" width="0.1524" layer="91"/>
<wire x1="203.2" y1="187.96" x2="203.2" y2="180.34" width="0.1524" layer="91"/>
<wire x1="203.2" y1="180.34" x2="208.28" y2="180.34" width="0.1524" layer="91"/>
<label x="213.36" y="180.34" size="1.778" layer="95"/>
<pinref part="D6" gate="G$1" pin="A"/>
<pinref part="C27" gate="C1" pin="2"/>
<wire x1="208.28" y1="180.34" x2="215.9" y2="180.34" width="0.1524" layer="91"/>
<wire x1="208.28" y1="182.88" x2="208.28" y2="180.34" width="0.1524" layer="91"/>
<junction x="208.28" y="180.34"/>
</segment>
<segment>
<pinref part="C" gate="G$1" pin="-"/>
<wire x1="127" y1="236.22" x2="127" y2="233.68" width="0.1524" layer="91"/>
<wire x1="127" y1="233.68" x2="121.92" y2="233.68" width="0.1524" layer="91"/>
<junction x="127" y="233.68"/>
<label x="121.92" y="233.68" size="1.778" layer="95"/>
<pinref part="C1" gate="C1" pin="2"/>
<wire x1="139.7" y1="236.22" x2="139.7" y2="233.68" width="0.1524" layer="91"/>
<pinref part="C6" gate="C1" pin="2"/>
<wire x1="139.7" y1="233.68" x2="147.32" y2="233.68" width="0.1524" layer="91"/>
<wire x1="147.32" y1="233.68" x2="154.94" y2="233.68" width="0.1524" layer="91"/>
<wire x1="154.94" y1="233.68" x2="162.56" y2="233.68" width="0.1524" layer="91"/>
<wire x1="162.56" y1="233.68" x2="170.18" y2="233.68" width="0.1524" layer="91"/>
<wire x1="170.18" y1="233.68" x2="182.88" y2="233.68" width="0.1524" layer="91"/>
<wire x1="182.88" y1="233.68" x2="182.88" y2="236.22" width="0.1524" layer="91"/>
<pinref part="C5" gate="C1" pin="2"/>
<wire x1="170.18" y1="236.22" x2="170.18" y2="233.68" width="0.1524" layer="91"/>
<junction x="170.18" y="233.68"/>
<pinref part="C4" gate="C1" pin="2"/>
<wire x1="162.56" y1="236.22" x2="162.56" y2="233.68" width="0.1524" layer="91"/>
<junction x="162.56" y="233.68"/>
<pinref part="C3" gate="C1" pin="2"/>
<wire x1="154.94" y1="236.22" x2="154.94" y2="233.68" width="0.1524" layer="91"/>
<junction x="154.94" y="233.68"/>
<pinref part="C2" gate="C1" pin="2"/>
<wire x1="147.32" y1="236.22" x2="147.32" y2="233.68" width="0.1524" layer="91"/>
<junction x="147.32" y="233.68"/>
<junction x="139.7" y="233.68"/>
<wire x1="127" y1="233.68" x2="139.7" y2="233.68" width="0.1524" layer="91"/>
<junction x="182.88" y="233.68"/>
<pinref part="C22" gate="C1" pin="2"/>
<wire x1="182.88" y1="233.68" x2="200.66" y2="233.68" width="0.1524" layer="91"/>
<wire x1="200.66" y1="233.68" x2="200.66" y2="236.22" width="0.1524" layer="91"/>
<pinref part="C23" gate="C1" pin="2"/>
<wire x1="200.66" y1="233.68" x2="210.82" y2="233.68" width="0.1524" layer="91"/>
<wire x1="210.82" y1="233.68" x2="210.82" y2="236.22" width="0.1524" layer="91"/>
<junction x="200.66" y="233.68"/>
</segment>
<segment>
<pinref part="V1" gate="G$1" pin="VSS"/>
<wire x1="139.7" y1="284.48" x2="129.54" y2="284.48" width="0.1524" layer="91"/>
<label x="121.92" y="284.48" size="1.778" layer="95"/>
<pinref part="C7" gate="C1" pin="2"/>
<wire x1="129.54" y1="284.48" x2="121.92" y2="284.48" width="0.1524" layer="91"/>
<junction x="129.54" y="284.48"/>
</segment>
<segment>
<pinref part="C8" gate="C1" pin="2"/>
<wire x1="162.56" y1="284.48" x2="162.56" y2="281.94" width="0.1524" layer="91"/>
<wire x1="162.56" y1="281.94" x2="182.88" y2="281.94" width="0.1524" layer="91"/>
<pinref part="C9" gate="C1" pin="2"/>
<wire x1="182.88" y1="281.94" x2="182.88" y2="284.48" width="0.1524" layer="91"/>
<pinref part="C10" gate="C1" pin="2"/>
<wire x1="182.88" y1="281.94" x2="203.2" y2="281.94" width="0.1524" layer="91"/>
<wire x1="203.2" y1="281.94" x2="203.2" y2="284.48" width="0.1524" layer="91"/>
<junction x="182.88" y="281.94"/>
<label x="172.72" y="281.94" size="1.778" layer="95"/>
<label x="210.82" y="281.94" size="1.778" layer="95"/>
<wire x1="203.2" y1="281.94" x2="213.36" y2="281.94" width="0.1524" layer="91"/>
<junction x="203.2" y="281.94"/>
</segment>
<segment>
<pinref part="V2" gate="G$1" pin="GND"/>
<wire x1="160.02" y1="256.54" x2="129.54" y2="256.54" width="0.1524" layer="91"/>
<pinref part="C11" gate="C1" pin="2"/>
<wire x1="129.54" y1="256.54" x2="121.92" y2="256.54" width="0.1524" layer="91"/>
<wire x1="129.54" y1="259.08" x2="129.54" y2="256.54" width="0.1524" layer="91"/>
<junction x="129.54" y="256.54"/>
<label x="121.92" y="256.54" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="R2" gate="G$1" pin="2"/>
<pinref part="C12" gate="C1" pin="2"/>
<wire x1="195.58" y1="256.54" x2="198.12" y2="256.54" width="0.1524" layer="91"/>
<wire x1="198.12" y1="256.54" x2="198.12" y2="259.08" width="0.1524" layer="91"/>
<wire x1="198.12" y1="256.54" x2="213.36" y2="256.54" width="0.1524" layer="91"/>
<junction x="198.12" y="256.54"/>
<label x="210.82" y="256.54" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="C18" gate="C1" pin="2"/>
<wire x1="208.28" y1="200.66" x2="208.28" y2="198.12" width="0.1524" layer="91"/>
<wire x1="208.28" y1="198.12" x2="193.04" y2="198.12" width="0.1524" layer="91"/>
<pinref part="C13" gate="C1" pin="2"/>
<wire x1="193.04" y1="198.12" x2="177.8" y2="198.12" width="0.1524" layer="91"/>
<wire x1="177.8" y1="198.12" x2="162.56" y2="198.12" width="0.1524" layer="91"/>
<wire x1="162.56" y1="198.12" x2="147.32" y2="198.12" width="0.1524" layer="91"/>
<wire x1="147.32" y1="198.12" x2="132.08" y2="198.12" width="0.1524" layer="91"/>
<wire x1="132.08" y1="198.12" x2="121.92" y2="198.12" width="0.1524" layer="91"/>
<wire x1="132.08" y1="200.66" x2="132.08" y2="198.12" width="0.1524" layer="91"/>
<junction x="132.08" y="198.12"/>
<pinref part="C14" gate="C1" pin="2"/>
<wire x1="147.32" y1="200.66" x2="147.32" y2="198.12" width="0.1524" layer="91"/>
<junction x="147.32" y="198.12"/>
<pinref part="C15" gate="C1" pin="2"/>
<wire x1="162.56" y1="200.66" x2="162.56" y2="198.12" width="0.1524" layer="91"/>
<junction x="162.56" y="198.12"/>
<pinref part="C16" gate="C1" pin="2"/>
<wire x1="177.8" y1="200.66" x2="177.8" y2="198.12" width="0.1524" layer="91"/>
<junction x="177.8" y="198.12"/>
<pinref part="C17" gate="C1" pin="2"/>
<wire x1="193.04" y1="200.66" x2="193.04" y2="198.12" width="0.1524" layer="91"/>
<junction x="193.04" y="198.12"/>
<label x="121.92" y="198.12" size="1.778" layer="95"/>
<wire x1="208.28" y1="198.12" x2="218.44" y2="198.12" width="0.1524" layer="91"/>
<junction x="208.28" y="198.12"/>
<label x="215.9" y="198.12" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="R14" gate="G$1" pin="1"/>
<wire x1="91.44" y1="12.7" x2="91.44" y2="10.16" width="0.1524" layer="91"/>
<wire x1="91.44" y1="10.16" x2="76.2" y2="10.16" width="0.1524" layer="91"/>
<pinref part="R9" gate="G$1" pin="1"/>
<wire x1="76.2" y1="10.16" x2="60.96" y2="10.16" width="0.1524" layer="91"/>
<wire x1="60.96" y1="10.16" x2="45.72" y2="10.16" width="0.1524" layer="91"/>
<wire x1="45.72" y1="10.16" x2="30.48" y2="10.16" width="0.1524" layer="91"/>
<wire x1="30.48" y1="10.16" x2="15.24" y2="10.16" width="0.1524" layer="91"/>
<wire x1="15.24" y1="10.16" x2="15.24" y2="12.7" width="0.1524" layer="91"/>
<pinref part="R10" gate="G$1" pin="1"/>
<wire x1="30.48" y1="12.7" x2="30.48" y2="10.16" width="0.1524" layer="91"/>
<junction x="30.48" y="10.16"/>
<pinref part="R11" gate="G$1" pin="1"/>
<wire x1="45.72" y1="12.7" x2="45.72" y2="10.16" width="0.1524" layer="91"/>
<junction x="45.72" y="10.16"/>
<pinref part="R12" gate="G$1" pin="1"/>
<wire x1="60.96" y1="12.7" x2="60.96" y2="10.16" width="0.1524" layer="91"/>
<junction x="60.96" y="10.16"/>
<pinref part="R13" gate="G$1" pin="1"/>
<wire x1="76.2" y1="12.7" x2="76.2" y2="10.16" width="0.1524" layer="91"/>
<junction x="76.2" y="10.16"/>
<wire x1="91.44" y1="10.16" x2="101.6" y2="10.16" width="0.1524" layer="91"/>
<junction x="91.44" y="10.16"/>
<wire x1="15.24" y1="10.16" x2="5.08" y2="10.16" width="0.1524" layer="91"/>
<junction x="15.24" y="10.16"/>
<label x="5.08" y="10.16" size="1.778" layer="95"/>
<label x="99.06" y="10.16" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="E0" gate="G$1" pin="C"/>
<wire x1="63.5" y1="132.08" x2="71.12" y2="132.08" width="0.1524" layer="91"/>
<label x="66.04" y="132.08" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="E1" gate="G$1" pin="C"/>
<wire x1="63.5" y1="116.84" x2="71.12" y2="116.84" width="0.1524" layer="91"/>
<label x="66.04" y="116.84" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="E2" gate="G$1" pin="C"/>
<wire x1="63.5" y1="101.6" x2="71.12" y2="101.6" width="0.1524" layer="91"/>
<label x="66.04" y="101.6" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="E3" gate="G$1" pin="C"/>
<wire x1="63.5" y1="81.28" x2="71.12" y2="81.28" width="0.1524" layer="91"/>
<label x="66.04" y="81.28" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="E4" gate="G$1" pin="C"/>
<wire x1="63.5" y1="66.04" x2="71.12" y2="66.04" width="0.1524" layer="91"/>
<label x="66.04" y="66.04" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="E5" gate="G$1" pin="C"/>
<wire x1="63.5" y1="50.8" x2="71.12" y2="50.8" width="0.1524" layer="91"/>
<label x="66.04" y="50.8" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="R18" gate="G$1" pin="1"/>
<wire x1="177.8" y1="60.96" x2="177.8" y2="58.42" width="0.1524" layer="91"/>
<wire x1="177.8" y1="58.42" x2="185.42" y2="58.42" width="0.1524" layer="91"/>
<pinref part="R20" gate="G$1" pin="1"/>
<wire x1="185.42" y1="58.42" x2="198.12" y2="58.42" width="0.1524" layer="91"/>
<wire x1="198.12" y1="58.42" x2="205.74" y2="58.42" width="0.1524" layer="91"/>
<wire x1="205.74" y1="58.42" x2="218.44" y2="58.42" width="0.1524" layer="91"/>
<wire x1="218.44" y1="58.42" x2="218.44" y2="60.96" width="0.1524" layer="91"/>
<pinref part="R19" gate="G$1" pin="1"/>
<wire x1="198.12" y1="60.96" x2="198.12" y2="58.42" width="0.1524" layer="91"/>
<junction x="198.12" y="58.42"/>
<wire x1="177.8" y1="58.42" x2="167.64" y2="58.42" width="0.1524" layer="91"/>
<junction x="177.8" y="58.42"/>
<label x="167.64" y="58.42" size="1.778" layer="95"/>
<pinref part="C31" gate="C1" pin="2"/>
<wire x1="185.42" y1="60.96" x2="185.42" y2="58.42" width="0.1524" layer="91"/>
<junction x="185.42" y="58.42"/>
<pinref part="C32" gate="C1" pin="2"/>
<wire x1="205.74" y1="60.96" x2="205.74" y2="58.42" width="0.1524" layer="91"/>
<junction x="205.74" y="58.42"/>
<pinref part="C33" gate="C1" pin="2"/>
<wire x1="226.06" y1="60.96" x2="226.06" y2="58.42" width="0.1524" layer="91"/>
<wire x1="226.06" y1="58.42" x2="218.44" y2="58.42" width="0.1524" layer="91"/>
<junction x="218.44" y="58.42"/>
</segment>
<segment>
<pinref part="Z1" gate="A0" pin="1"/>
<wire x1="91.44" y1="111.76" x2="88.9" y2="111.76" width="0.1524" layer="91"/>
<label x="83.82" y="111.76" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="Z1" gate="A1" pin="1"/>
<wire x1="91.44" y1="106.68" x2="88.9" y2="106.68" width="0.1524" layer="91"/>
<label x="83.82" y="106.68" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="Z2" gate="A1" pin="1"/>
<wire x1="91.44" y1="50.8" x2="88.9" y2="50.8" width="0.1524" layer="91"/>
<label x="83.82" y="50.8" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="R24" gate="G$1" pin="1"/>
<wire x1="243.84" y1="254" x2="243.84" y2="251.46" width="0.1524" layer="91"/>
<wire x1="243.84" y1="251.46" x2="236.22" y2="251.46" width="0.1524" layer="91"/>
<label x="236.22" y="251.46" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="R27" gate="G$1" pin="1"/>
<wire x1="243.84" y1="152.4" x2="243.84" y2="149.86" width="0.1524" layer="91"/>
<wire x1="243.84" y1="149.86" x2="236.22" y2="149.86" width="0.1524" layer="91"/>
<label x="236.22" y="149.86" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="R30" gate="G$1" pin="1"/>
<wire x1="243.84" y1="203.2" x2="243.84" y2="200.66" width="0.1524" layer="91"/>
<wire x1="243.84" y1="200.66" x2="236.22" y2="200.66" width="0.1524" layer="91"/>
<label x="236.22" y="200.66" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="R33" gate="G$1" pin="1"/>
<wire x1="243.84" y1="116.84" x2="243.84" y2="114.3" width="0.1524" layer="91"/>
<wire x1="243.84" y1="114.3" x2="236.22" y2="114.3" width="0.1524" layer="91"/>
<label x="236.22" y="114.3" size="1.778" layer="95"/>
<pinref part="C30" gate="C1" pin="2"/>
<wire x1="243.84" y1="114.3" x2="251.46" y2="114.3" width="0.1524" layer="91"/>
<wire x1="251.46" y1="114.3" x2="251.46" y2="116.84" width="0.1524" layer="91"/>
<junction x="243.84" y="114.3"/>
</segment>
<segment>
<pinref part="M2" gate="GND" pin="1"/>
<wire x1="215.9" y1="111.76" x2="205.74" y2="111.76" width="0.1524" layer="91"/>
<label x="205.74" y="111.76" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="Z1" gate="GND" pin="1"/>
<wire x1="91.44" y1="116.84" x2="88.9" y2="116.84" width="0.1524" layer="91"/>
<label x="83.82" y="116.84" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="Z2" gate="GND" pin="1"/>
<wire x1="91.44" y1="60.96" x2="88.9" y2="60.96" width="0.1524" layer="91"/>
<label x="83.82" y="60.96" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="RX0" gate="GND" pin="1"/>
<wire x1="142.24" y1="129.54" x2="139.7" y2="129.54" width="0.1524" layer="91"/>
<label x="134.62" y="129.54" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="RX1" gate="GND" pin="1"/>
<wire x1="142.24" y1="114.3" x2="139.7" y2="114.3" width="0.1524" layer="91"/>
<label x="134.62" y="114.3" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="RX2" gate="GND" pin="1"/>
<wire x1="142.24" y1="99.06" x2="139.7" y2="99.06" width="0.1524" layer="91"/>
<label x="134.62" y="99.06" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="RX3" gate="GND" pin="1"/>
<wire x1="142.24" y1="83.82" x2="139.7" y2="83.82" width="0.1524" layer="91"/>
<label x="134.62" y="83.82" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="RX4" gate="GND" pin="1"/>
<wire x1="142.24" y1="68.58" x2="139.7" y2="68.58" width="0.1524" layer="91"/>
<label x="134.62" y="68.58" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="RX5" gate="GND" pin="1"/>
<wire x1="142.24" y1="53.34" x2="139.7" y2="53.34" width="0.1524" layer="91"/>
<label x="134.62" y="53.34" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="A1" gate="GND" pin="1"/>
<wire x1="91.44" y1="96.52" x2="88.9" y2="96.52" width="0.1524" layer="91"/>
<label x="83.82" y="96.52" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="A2" gate="GND" pin="1"/>
<wire x1="91.44" y1="86.36" x2="88.9" y2="86.36" width="0.1524" layer="91"/>
<label x="83.82" y="86.36" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="R36" gate="G$1" pin="1"/>
<wire x1="251.46" y1="81.28" x2="251.46" y2="78.74" width="0.1524" layer="91"/>
<wire x1="251.46" y1="78.74" x2="243.84" y2="78.74" width="0.1524" layer="91"/>
<label x="243.84" y="78.74" size="1.778" layer="95"/>
<pinref part="C24" gate="C1" pin="2"/>
<wire x1="251.46" y1="78.74" x2="259.08" y2="78.74" width="0.1524" layer="91"/>
<wire x1="259.08" y1="78.74" x2="259.08" y2="81.28" width="0.1524" layer="91"/>
<junction x="251.46" y="78.74"/>
</segment>
<segment>
<pinref part="R38" gate="G$1" pin="1"/>
<wire x1="251.46" y1="45.72" x2="251.46" y2="43.18" width="0.1524" layer="91"/>
<wire x1="251.46" y1="43.18" x2="243.84" y2="43.18" width="0.1524" layer="91"/>
<label x="243.84" y="43.18" size="1.778" layer="95"/>
<pinref part="C28" gate="C1" pin="2"/>
<wire x1="251.46" y1="43.18" x2="259.08" y2="43.18" width="0.1524" layer="91"/>
<wire x1="259.08" y1="43.18" x2="259.08" y2="45.72" width="0.1524" layer="91"/>
<junction x="251.46" y="43.18"/>
</segment>
<segment>
<pinref part="R40" gate="G$1" pin="1"/>
<wire x1="251.46" y1="10.16" x2="251.46" y2="7.62" width="0.1524" layer="91"/>
<wire x1="251.46" y1="7.62" x2="243.84" y2="7.62" width="0.1524" layer="91"/>
<label x="243.84" y="7.62" size="1.778" layer="95"/>
<pinref part="C29" gate="C1" pin="2"/>
<wire x1="251.46" y1="7.62" x2="259.08" y2="7.62" width="0.1524" layer="91"/>
<wire x1="259.08" y1="7.62" x2="259.08" y2="10.16" width="0.1524" layer="91"/>
<junction x="251.46" y="7.62"/>
</segment>
<segment>
<pinref part="M1" gate="GND" pin="1"/>
<wire x1="215.9" y1="129.54" x2="205.74" y2="129.54" width="0.1524" layer="91"/>
<label x="205.74" y="129.54" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="MEM" gate="G$1" pin="VSS"/>
<wire x1="312.42" y1="294.64" x2="317.5" y2="294.64" width="0.1524" layer="91"/>
<label x="312.42" y="294.64" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="F1" gate="G$1" pin="S"/>
<wire x1="182.88" y1="7.62" x2="182.88" y2="5.08" width="0.1524" layer="91"/>
<pinref part="F2" gate="G$1" pin="S"/>
<wire x1="182.88" y1="5.08" x2="203.2" y2="5.08" width="0.1524" layer="91"/>
<wire x1="203.2" y1="5.08" x2="223.52" y2="5.08" width="0.1524" layer="91"/>
<wire x1="223.52" y1="5.08" x2="223.52" y2="7.62" width="0.1524" layer="91"/>
<pinref part="F1" gate="G$2" pin="S"/>
<wire x1="203.2" y1="7.62" x2="203.2" y2="5.08" width="0.1524" layer="91"/>
<junction x="203.2" y="5.08"/>
<wire x1="182.88" y1="5.08" x2="170.18" y2="5.08" width="0.1524" layer="91"/>
<junction x="182.88" y="5.08"/>
<label x="170.18" y="5.08" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="P2" gate="G$1" pin="2"/>
<wire x1="17.78" y1="182.88" x2="7.62" y2="182.88" width="0.1524" layer="91"/>
<label x="7.62" y="182.88" size="1.778" layer="95"/>
</segment>
</net>
<net name="PDI_CLK" class="0">
<segment>
<pinref part="UC" gate="G$1" pin="RESET"/>
<wire x1="22.86" y1="297.18" x2="5.08" y2="297.18" width="0.1524" layer="91"/>
<label x="5.08" y="297.18" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="R21" gate="G$1" pin="2"/>
<wire x1="139.7" y1="22.86" x2="147.32" y2="22.86" width="0.1524" layer="91"/>
<label x="142.24" y="22.86" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="P1" gate="G$1" pin="2"/>
<wire x1="17.78" y1="198.12" x2="7.62" y2="198.12" width="0.1524" layer="91"/>
<label x="7.62" y="198.12" size="1.778" layer="95"/>
</segment>
</net>
<net name="PDI_DATA" class="0">
<segment>
<pinref part="UC" gate="G$1" pin="PDI_DATA"/>
<wire x1="22.86" y1="294.64" x2="5.08" y2="294.64" width="0.1524" layer="91"/>
<label x="5.08" y="294.64" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="P1" gate="G$1" pin="1"/>
<wire x1="17.78" y1="200.66" x2="7.62" y2="200.66" width="0.1524" layer="91"/>
<label x="7.62" y="200.66" size="1.778" layer="95"/>
</segment>
</net>
<net name="VIN_SEN" class="0">
<segment>
<pinref part="R34" gate="G$1" pin="1"/>
<pinref part="R33" gate="G$1" pin="2"/>
<wire x1="243.84" y1="132.08" x2="243.84" y2="129.54" width="0.1524" layer="91"/>
<wire x1="243.84" y1="129.54" x2="243.84" y2="127" width="0.1524" layer="91"/>
<wire x1="243.84" y1="129.54" x2="251.46" y2="129.54" width="0.1524" layer="91"/>
<junction x="243.84" y="129.54"/>
<label x="248.92" y="129.54" size="1.778" layer="95"/>
<pinref part="C30" gate="C1" pin="1"/>
<wire x1="251.46" y1="129.54" x2="259.08" y2="129.54" width="0.1524" layer="91"/>
<wire x1="251.46" y1="124.46" x2="251.46" y2="129.54" width="0.1524" layer="91"/>
<junction x="251.46" y="129.54"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PB1"/>
<wire x1="88.9" y1="271.78" x2="106.68" y2="271.78" width="0.1524" layer="91"/>
<label x="93.98" y="271.78" size="1.778" layer="95"/>
</segment>
</net>
<net name="LEG1_SEN" class="0">
<segment>
<pinref part="R25" gate="G$1" pin="1"/>
<pinref part="R24" gate="G$1" pin="2"/>
<wire x1="243.84" y1="269.24" x2="243.84" y2="266.7" width="0.1524" layer="91"/>
<wire x1="243.84" y1="266.7" x2="243.84" y2="264.16" width="0.1524" layer="91"/>
<wire x1="243.84" y1="266.7" x2="259.08" y2="266.7" width="0.1524" layer="91"/>
<junction x="243.84" y="266.7"/>
<label x="246.38" y="266.7" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PA2"/>
<wire x1="88.9" y1="292.1" x2="106.68" y2="292.1" width="0.1524" layer="91"/>
<label x="93.98" y="292.1" size="1.778" layer="95"/>
</segment>
</net>
<net name="LEG2_SEN" class="0">
<segment>
<pinref part="R31" gate="G$1" pin="1"/>
<pinref part="R30" gate="G$1" pin="2"/>
<wire x1="243.84" y1="218.44" x2="243.84" y2="215.9" width="0.1524" layer="91"/>
<wire x1="243.84" y1="215.9" x2="243.84" y2="213.36" width="0.1524" layer="91"/>
<wire x1="243.84" y1="215.9" x2="259.08" y2="215.9" width="0.1524" layer="91"/>
<junction x="243.84" y="215.9"/>
<label x="246.38" y="215.9" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PA3"/>
<wire x1="88.9" y1="289.56" x2="106.68" y2="289.56" width="0.1524" layer="91"/>
<label x="93.98" y="289.56" size="1.778" layer="95"/>
</segment>
</net>
<net name="LEG3_SEN" class="0">
<segment>
<pinref part="R28" gate="G$1" pin="1"/>
<pinref part="R27" gate="G$1" pin="2"/>
<wire x1="243.84" y1="167.64" x2="243.84" y2="165.1" width="0.1524" layer="91"/>
<wire x1="243.84" y1="165.1" x2="243.84" y2="162.56" width="0.1524" layer="91"/>
<wire x1="243.84" y1="165.1" x2="259.08" y2="165.1" width="0.1524" layer="91"/>
<junction x="243.84" y="165.1"/>
<label x="246.38" y="165.1" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PA4"/>
<wire x1="88.9" y1="287.02" x2="106.68" y2="287.02" width="0.1524" layer="91"/>
<label x="93.98" y="287.02" size="1.778" layer="95"/>
</segment>
</net>
<net name="R_SEN" class="0">
<segment>
<pinref part="PD_R" gate="G$1" pin="A"/>
<pinref part="R18" gate="G$1" pin="2"/>
<wire x1="177.8" y1="76.2" x2="177.8" y2="73.66" width="0.1524" layer="91"/>
<wire x1="177.8" y1="73.66" x2="177.8" y2="71.12" width="0.1524" layer="91"/>
<wire x1="177.8" y1="73.66" x2="185.42" y2="73.66" width="0.1524" layer="91"/>
<junction x="177.8" y="73.66"/>
<label x="182.88" y="73.66" size="1.778" layer="95"/>
<pinref part="C31" gate="C1" pin="1"/>
<wire x1="185.42" y1="73.66" x2="190.5" y2="73.66" width="0.1524" layer="91"/>
<wire x1="185.42" y1="68.58" x2="185.42" y2="73.66" width="0.1524" layer="91"/>
<junction x="185.42" y="73.66"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PA5"/>
<wire x1="88.9" y1="284.48" x2="106.68" y2="284.48" width="0.1524" layer="91"/>
<label x="99.06" y="284.48" size="1.778" layer="95"/>
</segment>
</net>
<net name="G_SEN" class="0">
<segment>
<pinref part="UC" gate="G$1" pin="PA6"/>
<wire x1="88.9" y1="281.94" x2="106.68" y2="281.94" width="0.1524" layer="91"/>
<label x="99.06" y="281.94" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="PD_B" gate="G$1" pin="A"/>
<pinref part="R20" gate="G$1" pin="2"/>
<wire x1="218.44" y1="76.2" x2="218.44" y2="73.66" width="0.1524" layer="91"/>
<wire x1="218.44" y1="73.66" x2="218.44" y2="71.12" width="0.1524" layer="91"/>
<wire x1="218.44" y1="73.66" x2="226.06" y2="73.66" width="0.1524" layer="91"/>
<junction x="218.44" y="73.66"/>
<label x="223.52" y="73.66" size="1.778" layer="95"/>
<pinref part="C33" gate="C1" pin="1"/>
<wire x1="226.06" y1="73.66" x2="231.14" y2="73.66" width="0.1524" layer="91"/>
<wire x1="226.06" y1="68.58" x2="226.06" y2="73.66" width="0.1524" layer="91"/>
<junction x="226.06" y="73.66"/>
</segment>
</net>
<net name="B_SEN" class="0">
<segment>
<pinref part="PD_G" gate="G$1" pin="A"/>
<pinref part="R19" gate="G$1" pin="2"/>
<wire x1="198.12" y1="76.2" x2="198.12" y2="73.66" width="0.1524" layer="91"/>
<wire x1="198.12" y1="73.66" x2="198.12" y2="71.12" width="0.1524" layer="91"/>
<wire x1="198.12" y1="73.66" x2="205.74" y2="73.66" width="0.1524" layer="91"/>
<junction x="198.12" y="73.66"/>
<label x="203.2" y="73.66" size="1.778" layer="95"/>
<pinref part="C32" gate="C1" pin="1"/>
<wire x1="205.74" y1="73.66" x2="210.82" y2="73.66" width="0.1524" layer="91"/>
<wire x1="205.74" y1="68.58" x2="205.74" y2="73.66" width="0.1524" layer="91"/>
<junction x="205.74" y="73.66"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PA7"/>
<wire x1="88.9" y1="279.4" x2="106.68" y2="279.4" width="0.1524" layer="91"/>
<label x="99.06" y="279.4" size="1.778" layer="95"/>
</segment>
</net>
<net name="IR0_SEN" class="0">
<segment>
<pinref part="S0" gate="G$1" pin="A"/>
<pinref part="R9" gate="G$1" pin="2"/>
<wire x1="15.24" y1="27.94" x2="15.24" y2="25.4" width="0.1524" layer="91"/>
<wire x1="15.24" y1="25.4" x2="15.24" y2="22.86" width="0.1524" layer="91"/>
<wire x1="15.24" y1="25.4" x2="25.4" y2="25.4" width="0.1524" layer="91"/>
<junction x="15.24" y="25.4"/>
<label x="17.78" y="25.4" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PB5"/>
<wire x1="88.9" y1="261.62" x2="106.68" y2="261.62" width="0.1524" layer="91"/>
<label x="96.52" y="261.62" size="1.778" layer="95"/>
</segment>
</net>
<net name="IR1_SEN" class="0">
<segment>
<pinref part="S1" gate="G$1" pin="A"/>
<pinref part="R10" gate="G$1" pin="2"/>
<wire x1="30.48" y1="27.94" x2="30.48" y2="25.4" width="0.1524" layer="91"/>
<wire x1="30.48" y1="25.4" x2="30.48" y2="22.86" width="0.1524" layer="91"/>
<wire x1="30.48" y1="25.4" x2="40.64" y2="25.4" width="0.1524" layer="91"/>
<junction x="30.48" y="25.4"/>
<label x="33.02" y="25.4" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PB6"/>
<wire x1="88.9" y1="259.08" x2="106.68" y2="259.08" width="0.1524" layer="91"/>
<label x="96.52" y="259.08" size="1.778" layer="95"/>
</segment>
</net>
<net name="IR2_SEN" class="0">
<segment>
<pinref part="S2" gate="G$1" pin="A"/>
<pinref part="R11" gate="G$1" pin="2"/>
<wire x1="45.72" y1="27.94" x2="45.72" y2="25.4" width="0.1524" layer="91"/>
<wire x1="45.72" y1="25.4" x2="45.72" y2="22.86" width="0.1524" layer="91"/>
<wire x1="45.72" y1="25.4" x2="55.88" y2="25.4" width="0.1524" layer="91"/>
<junction x="45.72" y="25.4"/>
<label x="48.26" y="25.4" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PB7"/>
<wire x1="88.9" y1="256.54" x2="106.68" y2="256.54" width="0.1524" layer="91"/>
<label x="96.52" y="256.54" size="1.778" layer="95"/>
</segment>
</net>
<net name="IR3_SEN" class="0">
<segment>
<pinref part="S3" gate="G$1" pin="A"/>
<pinref part="R12" gate="G$1" pin="2"/>
<wire x1="60.96" y1="27.94" x2="60.96" y2="25.4" width="0.1524" layer="91"/>
<wire x1="60.96" y1="25.4" x2="60.96" y2="22.86" width="0.1524" layer="91"/>
<wire x1="60.96" y1="25.4" x2="71.12" y2="25.4" width="0.1524" layer="91"/>
<junction x="60.96" y="25.4"/>
<label x="63.5" y="25.4" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PB4"/>
<wire x1="88.9" y1="264.16" x2="106.68" y2="264.16" width="0.1524" layer="91"/>
<label x="96.52" y="264.16" size="1.778" layer="95"/>
</segment>
</net>
<net name="IR4_SEN" class="0">
<segment>
<pinref part="S4" gate="G$1" pin="A"/>
<pinref part="R13" gate="G$1" pin="2"/>
<wire x1="76.2" y1="27.94" x2="76.2" y2="25.4" width="0.1524" layer="91"/>
<wire x1="76.2" y1="25.4" x2="76.2" y2="22.86" width="0.1524" layer="91"/>
<wire x1="76.2" y1="25.4" x2="86.36" y2="25.4" width="0.1524" layer="91"/>
<junction x="76.2" y="25.4"/>
<label x="78.74" y="25.4" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PB2"/>
<wire x1="88.9" y1="269.24" x2="106.68" y2="269.24" width="0.1524" layer="91"/>
<label x="96.52" y="269.24" size="1.778" layer="95"/>
</segment>
</net>
<net name="IR5_SEN" class="0">
<segment>
<pinref part="S5" gate="G$1" pin="A"/>
<pinref part="R14" gate="G$1" pin="2"/>
<wire x1="91.44" y1="27.94" x2="91.44" y2="25.4" width="0.1524" layer="91"/>
<wire x1="91.44" y1="25.4" x2="91.44" y2="22.86" width="0.1524" layer="91"/>
<wire x1="91.44" y1="25.4" x2="101.6" y2="25.4" width="0.1524" layer="91"/>
<junction x="91.44" y="25.4"/>
<label x="93.98" y="25.4" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PB3"/>
<wire x1="88.9" y1="266.7" x2="106.68" y2="266.7" width="0.1524" layer="91"/>
<label x="96.52" y="266.7" size="1.778" layer="95"/>
</segment>
</net>
<net name="SDA" class="0">
<segment>
<pinref part="R22" gate="G$1" pin="2"/>
<wire x1="139.7" y1="15.24" x2="147.32" y2="15.24" width="0.1524" layer="91"/>
<label x="142.24" y="15.24" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="Z1" gate="SDA" pin="1"/>
<wire x1="91.44" y1="132.08" x2="88.9" y2="132.08" width="0.1524" layer="91"/>
<label x="83.82" y="132.08" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="Z2" gate="SDA" pin="1"/>
<wire x1="91.44" y1="76.2" x2="88.9" y2="76.2" width="0.1524" layer="91"/>
<label x="83.82" y="76.2" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PD0"/>
<wire x1="88.9" y1="228.6" x2="96.52" y2="228.6" width="0.1524" layer="91"/>
<label x="91.44" y="228.6" size="1.778" layer="95"/>
</segment>
</net>
<net name="SCL" class="0">
<segment>
<pinref part="R23" gate="G$1" pin="2"/>
<wire x1="139.7" y1="7.62" x2="147.32" y2="7.62" width="0.1524" layer="91"/>
<label x="142.24" y="7.62" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="Z1" gate="SCL" pin="1"/>
<wire x1="91.44" y1="127" x2="88.9" y2="127" width="0.1524" layer="91"/>
<label x="83.82" y="127" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="Z2" gate="SCL" pin="1"/>
<wire x1="91.44" y1="71.12" x2="88.9" y2="71.12" width="0.1524" layer="91"/>
<label x="83.82" y="71.12" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PD1"/>
<wire x1="88.9" y1="226.06" x2="96.52" y2="226.06" width="0.1524" layer="91"/>
<label x="91.44" y="226.06" size="1.778" layer="95"/>
</segment>
</net>
<net name="RX0" class="0">
<segment>
<pinref part="UC" gate="G$1" pin="PC2"/>
<wire x1="88.9" y1="246.38" x2="106.68" y2="246.38" width="0.1524" layer="91"/>
<label x="101.6" y="246.38" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="RX0" gate="G$1" pin="C"/>
<wire x1="132.08" y1="137.16" x2="142.24" y2="137.16" width="0.1524" layer="91"/>
<label x="137.16" y="137.16" size="1.778" layer="95"/>
</segment>
</net>
<net name="TX0" class="0">
<segment>
<pinref part="UC" gate="G$1" pin="PC3"/>
<wire x1="88.9" y1="243.84" x2="106.68" y2="243.84" width="0.1524" layer="91"/>
<label x="101.6" y="243.84" size="1.778" layer="95"/>
</segment>
<segment>
<wire x1="20.32" y1="134.62" x2="10.16" y2="134.62" width="0.1524" layer="91"/>
<label x="10.16" y="134.62" size="1.778" layer="95"/>
<pinref part="A1" gate="A" pin="I0"/>
</segment>
</net>
<net name="M1A" class="0">
<segment>
<pinref part="M1" gate="G$2" pin="IN2"/>
<wire x1="170.18" y1="134.62" x2="165.1" y2="134.62" width="0.1524" layer="91"/>
<label x="165.1" y="134.62" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PC0"/>
<wire x1="88.9" y1="251.46" x2="101.6" y2="251.46" width="0.1524" layer="91"/>
<label x="96.52" y="251.46" size="1.778" layer="95"/>
</segment>
</net>
<net name="M1B" class="0">
<segment>
<pinref part="M1" gate="G$2" pin="IN1"/>
<wire x1="170.18" y1="144.78" x2="165.1" y2="144.78" width="0.1524" layer="91"/>
<label x="165.1" y="144.78" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PC1"/>
<wire x1="88.9" y1="248.92" x2="101.6" y2="248.92" width="0.1524" layer="91"/>
<label x="96.52" y="248.92" size="1.778" layer="95"/>
</segment>
</net>
<net name="RX1" class="0">
<segment>
<pinref part="UC" gate="G$1" pin="PC6"/>
<wire x1="88.9" y1="236.22" x2="106.68" y2="236.22" width="0.1524" layer="91"/>
<label x="101.6" y="236.22" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="RX1" gate="G$1" pin="C"/>
<wire x1="132.08" y1="121.92" x2="142.24" y2="121.92" width="0.1524" layer="91"/>
<label x="137.16" y="121.92" size="1.778" layer="95"/>
</segment>
</net>
<net name="TX1" class="0">
<segment>
<pinref part="UC" gate="G$1" pin="PC7"/>
<wire x1="88.9" y1="233.68" x2="106.68" y2="233.68" width="0.1524" layer="91"/>
<label x="101.6" y="233.68" size="1.778" layer="95"/>
</segment>
<segment>
<wire x1="20.32" y1="119.38" x2="10.16" y2="119.38" width="0.1524" layer="91"/>
<label x="10.16" y="119.38" size="1.778" layer="95"/>
<pinref part="A1" gate="B" pin="I0"/>
</segment>
</net>
<net name="CAR0" class="0">
<segment>
<wire x1="20.32" y1="129.54" x2="10.16" y2="129.54" width="0.1524" layer="91"/>
<label x="10.16" y="129.54" size="1.778" layer="95"/>
<pinref part="A1" gate="A" pin="I1"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PF0"/>
<wire x1="88.9" y1="182.88" x2="99.06" y2="182.88" width="0.1524" layer="91"/>
<label x="91.44" y="182.88" size="1.778" layer="95"/>
</segment>
</net>
<net name="CAR1" class="0">
<segment>
<wire x1="20.32" y1="114.3" x2="10.16" y2="114.3" width="0.1524" layer="91"/>
<label x="10.16" y="114.3" size="1.778" layer="95"/>
<pinref part="A1" gate="B" pin="I1"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PF1"/>
<wire x1="88.9" y1="180.34" x2="99.06" y2="180.34" width="0.1524" layer="91"/>
<label x="91.44" y="180.34" size="1.778" layer="95"/>
</segment>
</net>
<net name="RX2" class="0">
<segment>
<pinref part="UC" gate="G$1" pin="PD2"/>
<wire x1="88.9" y1="223.52" x2="106.68" y2="223.52" width="0.1524" layer="91"/>
<label x="101.6" y="223.52" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="RX2" gate="G$1" pin="C"/>
<wire x1="132.08" y1="106.68" x2="142.24" y2="106.68" width="0.1524" layer="91"/>
<label x="137.16" y="106.68" size="1.778" layer="95"/>
</segment>
</net>
<net name="TX2" class="0">
<segment>
<pinref part="UC" gate="G$1" pin="PD3"/>
<wire x1="88.9" y1="220.98" x2="106.68" y2="220.98" width="0.1524" layer="91"/>
<label x="101.6" y="220.98" size="1.778" layer="95"/>
</segment>
<segment>
<wire x1="20.32" y1="104.14" x2="10.16" y2="104.14" width="0.1524" layer="91"/>
<label x="10.16" y="104.14" size="1.778" layer="95"/>
<pinref part="A1" gate="C" pin="I0"/>
</segment>
</net>
<net name="RX3" class="0">
<segment>
<pinref part="RX3" gate="G$1" pin="C"/>
<wire x1="132.08" y1="91.44" x2="142.24" y2="91.44" width="0.1524" layer="91"/>
<label x="137.16" y="91.44" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PE2"/>
<wire x1="88.9" y1="200.66" x2="106.68" y2="200.66" width="0.1524" layer="91"/>
<label x="101.6" y="200.66" size="1.778" layer="95"/>
</segment>
</net>
<net name="TX3" class="0">
<segment>
<wire x1="20.32" y1="83.82" x2="10.16" y2="83.82" width="0.1524" layer="91"/>
<label x="10.16" y="83.82" size="1.778" layer="95"/>
<pinref part="A2" gate="B" pin="I0"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PE3"/>
<wire x1="88.9" y1="198.12" x2="106.68" y2="198.12" width="0.1524" layer="91"/>
<label x="101.6" y="198.12" size="1.778" layer="95"/>
</segment>
</net>
<net name="M2A" class="0">
<segment>
<pinref part="M1" gate="G$1" pin="IN2"/>
<wire x1="170.18" y1="116.84" x2="165.1" y2="116.84" width="0.1524" layer="91"/>
<label x="165.1" y="116.84" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PC4"/>
<wire x1="88.9" y1="241.3" x2="101.6" y2="241.3" width="0.1524" layer="91"/>
<label x="96.52" y="241.3" size="1.778" layer="95"/>
</segment>
</net>
<net name="M2B" class="0">
<segment>
<pinref part="M1" gate="G$1" pin="IN1"/>
<wire x1="170.18" y1="127" x2="165.1" y2="127" width="0.1524" layer="91"/>
<label x="165.1" y="127" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PC5"/>
<wire x1="88.9" y1="238.76" x2="101.6" y2="238.76" width="0.1524" layer="91"/>
<label x="96.52" y="238.76" size="1.778" layer="95"/>
</segment>
</net>
<net name="CAR2" class="0">
<segment>
<wire x1="20.32" y1="99.06" x2="10.16" y2="99.06" width="0.1524" layer="91"/>
<label x="10.16" y="99.06" size="1.778" layer="95"/>
<pinref part="A1" gate="C" pin="I1"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PF4"/>
<wire x1="88.9" y1="172.72" x2="99.06" y2="172.72" width="0.1524" layer="91"/>
<label x="91.44" y="172.72" size="1.778" layer="95"/>
</segment>
</net>
<net name="CAR3" class="0">
<segment>
<wire x1="20.32" y1="78.74" x2="10.16" y2="78.74" width="0.1524" layer="91"/>
<label x="10.16" y="78.74" size="1.778" layer="95"/>
<pinref part="A2" gate="B" pin="I1"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PF5"/>
<wire x1="88.9" y1="170.18" x2="99.06" y2="170.18" width="0.1524" layer="91"/>
<label x="91.44" y="170.18" size="1.778" layer="95"/>
</segment>
</net>
<net name="RX4" class="0">
<segment>
<pinref part="RX4" gate="G$1" pin="C"/>
<wire x1="132.08" y1="76.2" x2="142.24" y2="76.2" width="0.1524" layer="91"/>
<label x="137.16" y="76.2" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PE6"/>
<wire x1="88.9" y1="190.5" x2="106.68" y2="190.5" width="0.1524" layer="91"/>
<label x="101.6" y="190.5" size="1.778" layer="95"/>
</segment>
</net>
<net name="TX4" class="0">
<segment>
<wire x1="20.32" y1="68.58" x2="10.16" y2="68.58" width="0.1524" layer="91"/>
<label x="10.16" y="68.58" size="1.778" layer="95"/>
<pinref part="A2" gate="C" pin="I0"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PE7"/>
<wire x1="88.9" y1="187.96" x2="106.68" y2="187.96" width="0.1524" layer="91"/>
<label x="101.6" y="187.96" size="1.778" layer="95"/>
</segment>
</net>
<net name="M3A" class="0">
<segment>
<pinref part="M2" gate="G$2" pin="IN2"/>
<wire x1="170.18" y1="99.06" x2="165.1" y2="99.06" width="0.1524" layer="91"/>
<label x="165.1" y="99.06" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PE0"/>
<wire x1="88.9" y1="205.74" x2="101.6" y2="205.74" width="0.1524" layer="91"/>
<label x="96.52" y="205.74" size="1.778" layer="95"/>
</segment>
</net>
<net name="M3B" class="0">
<segment>
<pinref part="M2" gate="G$2" pin="IN1"/>
<wire x1="170.18" y1="109.22" x2="165.1" y2="109.22" width="0.1524" layer="91"/>
<label x="165.1" y="109.22" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PE1"/>
<wire x1="88.9" y1="203.2" x2="101.6" y2="203.2" width="0.1524" layer="91"/>
<label x="96.52" y="203.2" size="1.778" layer="95"/>
</segment>
</net>
<net name="RX5" class="0">
<segment>
<pinref part="RX5" gate="G$1" pin="C"/>
<wire x1="132.08" y1="60.96" x2="142.24" y2="60.96" width="0.1524" layer="91"/>
<label x="137.16" y="60.96" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PF2"/>
<wire x1="88.9" y1="177.8" x2="106.68" y2="177.8" width="0.1524" layer="91"/>
<label x="101.6" y="177.8" size="1.778" layer="95"/>
</segment>
</net>
<net name="TX5" class="0">
<segment>
<wire x1="20.32" y1="53.34" x2="10.16" y2="53.34" width="0.1524" layer="91"/>
<label x="10.16" y="53.34" size="1.778" layer="95"/>
<pinref part="A2" gate="A" pin="I0"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PF3"/>
<wire x1="88.9" y1="175.26" x2="106.68" y2="175.26" width="0.1524" layer="91"/>
<label x="101.6" y="175.26" size="1.778" layer="95"/>
</segment>
</net>
<net name="CAR4" class="0">
<segment>
<wire x1="20.32" y1="63.5" x2="10.16" y2="63.5" width="0.1524" layer="91"/>
<label x="10.16" y="63.5" size="1.778" layer="95"/>
<pinref part="A2" gate="C" pin="I1"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PF7"/>
<wire x1="88.9" y1="165.1" x2="99.06" y2="165.1" width="0.1524" layer="91"/>
<label x="91.44" y="165.1" size="1.778" layer="95"/>
</segment>
</net>
<net name="CAR5" class="0">
<segment>
<wire x1="20.32" y1="48.26" x2="10.16" y2="48.26" width="0.1524" layer="91"/>
<label x="10.16" y="48.26" size="1.778" layer="95"/>
<pinref part="A2" gate="A" pin="I1"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PF6"/>
<wire x1="88.9" y1="167.64" x2="99.06" y2="167.64" width="0.1524" layer="91"/>
<label x="91.44" y="167.64" size="1.778" layer="95"/>
</segment>
</net>
<net name="TX_!RST" class="0">
<segment>
<pinref part="Z2" gate="!RST" pin="1"/>
<wire x1="91.44" y1="45.72" x2="88.9" y2="45.72" width="0.1524" layer="91"/>
<label x="78.74" y="45.72" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="Z1" gate="!RST" pin="1"/>
<wire x1="91.44" y1="101.6" x2="88.9" y2="101.6" width="0.1524" layer="91"/>
<label x="78.74" y="101.6" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PR0"/>
<wire x1="22.86" y1="226.06" x2="7.62" y2="226.06" width="0.1524" layer="91"/>
<label x="7.62" y="226.06" size="1.778" layer="95"/>
</segment>
</net>
<net name="LEG1" class="0">
<segment>
<pinref part="LEG1" gate="G$1" pin="1"/>
<wire x1="127" y1="185.42" x2="127" y2="187.96" width="0.1524" layer="91"/>
<wire x1="127" y1="187.96" x2="127" y2="193.04" width="0.1524" layer="91"/>
<wire x1="127" y1="193.04" x2="129.54" y2="193.04" width="0.1524" layer="91"/>
<wire x1="129.54" y1="187.96" x2="127" y2="187.96" width="0.1524" layer="91"/>
<junction x="127" y="187.96"/>
<label x="129.54" y="180.34" size="1.778" layer="95"/>
<pinref part="D1" gate="G$1" pin="A"/>
<pinref part="D2" gate="G$1" pin="C"/>
</segment>
<segment>
<pinref part="R26" gate="G$1" pin="1"/>
<pinref part="R25" gate="G$1" pin="2"/>
<wire x1="243.84" y1="284.48" x2="243.84" y2="281.94" width="0.1524" layer="91"/>
<wire x1="243.84" y1="281.94" x2="243.84" y2="279.4" width="0.1524" layer="91"/>
<wire x1="243.84" y1="281.94" x2="236.22" y2="281.94" width="0.1524" layer="91"/>
<junction x="243.84" y="281.94"/>
<label x="236.22" y="281.94" size="1.778" layer="95"/>
</segment>
</net>
<net name="VIN" class="0">
<segment>
<wire x1="134.62" y1="193.04" x2="142.24" y2="193.04" width="0.1524" layer="91"/>
<label x="147.32" y="193.04" size="1.778" layer="95"/>
<pinref part="D1" gate="G$1" pin="C"/>
<pinref part="C25" gate="C1" pin="1"/>
<wire x1="142.24" y1="193.04" x2="149.86" y2="193.04" width="0.1524" layer="91"/>
<wire x1="142.24" y1="190.5" x2="142.24" y2="193.04" width="0.1524" layer="91"/>
<junction x="142.24" y="193.04"/>
</segment>
<segment>
<wire x1="167.64" y1="193.04" x2="175.26" y2="193.04" width="0.1524" layer="91"/>
<label x="180.34" y="193.04" size="1.778" layer="95"/>
<pinref part="D3" gate="G$1" pin="C"/>
<pinref part="C26" gate="C1" pin="1"/>
<wire x1="175.26" y1="193.04" x2="182.88" y2="193.04" width="0.1524" layer="91"/>
<wire x1="175.26" y1="190.5" x2="175.26" y2="193.04" width="0.1524" layer="91"/>
<junction x="175.26" y="193.04"/>
</segment>
<segment>
<wire x1="200.66" y1="193.04" x2="208.28" y2="193.04" width="0.1524" layer="91"/>
<label x="213.36" y="193.04" size="1.778" layer="95"/>
<pinref part="D5" gate="G$1" pin="C"/>
<pinref part="C27" gate="C1" pin="1"/>
<wire x1="208.28" y1="193.04" x2="215.9" y2="193.04" width="0.1524" layer="91"/>
<wire x1="208.28" y1="190.5" x2="208.28" y2="193.04" width="0.1524" layer="91"/>
<junction x="208.28" y="193.04"/>
</segment>
<segment>
<pinref part="C" gate="G$1" pin="+"/>
<wire x1="127" y1="243.84" x2="127" y2="246.38" width="0.1524" layer="91"/>
<wire x1="127" y1="246.38" x2="121.92" y2="246.38" width="0.1524" layer="91"/>
<label x="121.92" y="246.38" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="V2" gate="G$1" pin="VIN"/>
<wire x1="160.02" y1="271.78" x2="157.48" y2="271.78" width="0.1524" layer="91"/>
<pinref part="V2" gate="G$1" pin="EN"/>
<wire x1="157.48" y1="271.78" x2="139.7" y2="271.78" width="0.1524" layer="91"/>
<wire x1="139.7" y1="271.78" x2="129.54" y2="271.78" width="0.1524" layer="91"/>
<wire x1="129.54" y1="271.78" x2="121.92" y2="271.78" width="0.1524" layer="91"/>
<wire x1="160.02" y1="266.7" x2="157.48" y2="266.7" width="0.1524" layer="91"/>
<wire x1="157.48" y1="266.7" x2="157.48" y2="271.78" width="0.1524" layer="91"/>
<junction x="157.48" y="271.78"/>
<pinref part="L3" gate="G$1" pin="1"/>
<wire x1="142.24" y1="261.62" x2="139.7" y2="261.62" width="0.1524" layer="91"/>
<wire x1="139.7" y1="261.62" x2="139.7" y2="271.78" width="0.1524" layer="91"/>
<junction x="139.7" y="271.78"/>
<pinref part="C11" gate="C1" pin="1"/>
<wire x1="129.54" y1="266.7" x2="129.54" y2="271.78" width="0.1524" layer="91"/>
<junction x="129.54" y="271.78"/>
<label x="121.92" y="271.78" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="R34" gate="G$1" pin="2"/>
<wire x1="243.84" y1="142.24" x2="243.84" y2="144.78" width="0.1524" layer="91"/>
<wire x1="243.84" y1="144.78" x2="236.22" y2="144.78" width="0.1524" layer="91"/>
<label x="236.22" y="144.78" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="P2" gate="G$1" pin="1"/>
<wire x1="17.78" y1="185.42" x2="7.62" y2="185.42" width="0.1524" layer="91"/>
<label x="7.62" y="185.42" size="1.778" layer="95"/>
</segment>
</net>
<net name="LEG2" class="0">
<segment>
<pinref part="LEG2" gate="G$1" pin="1"/>
<wire x1="160.02" y1="185.42" x2="160.02" y2="187.96" width="0.1524" layer="91"/>
<wire x1="160.02" y1="187.96" x2="160.02" y2="193.04" width="0.1524" layer="91"/>
<wire x1="160.02" y1="193.04" x2="162.56" y2="193.04" width="0.1524" layer="91"/>
<wire x1="162.56" y1="187.96" x2="160.02" y2="187.96" width="0.1524" layer="91"/>
<junction x="160.02" y="187.96"/>
<label x="162.56" y="180.34" size="1.778" layer="95"/>
<pinref part="D3" gate="G$1" pin="A"/>
<pinref part="D4" gate="G$1" pin="C"/>
</segment>
<segment>
<pinref part="R32" gate="G$1" pin="1"/>
<pinref part="R31" gate="G$1" pin="2"/>
<wire x1="243.84" y1="233.68" x2="243.84" y2="231.14" width="0.1524" layer="91"/>
<wire x1="243.84" y1="231.14" x2="243.84" y2="228.6" width="0.1524" layer="91"/>
<wire x1="243.84" y1="231.14" x2="236.22" y2="231.14" width="0.1524" layer="91"/>
<junction x="243.84" y="231.14"/>
<label x="236.22" y="231.14" size="1.778" layer="95"/>
</segment>
</net>
<net name="LEG3" class="0">
<segment>
<pinref part="LEG3" gate="G$1" pin="1"/>
<wire x1="193.04" y1="185.42" x2="193.04" y2="187.96" width="0.1524" layer="91"/>
<wire x1="193.04" y1="187.96" x2="193.04" y2="193.04" width="0.1524" layer="91"/>
<wire x1="193.04" y1="193.04" x2="195.58" y2="193.04" width="0.1524" layer="91"/>
<wire x1="195.58" y1="187.96" x2="193.04" y2="187.96" width="0.1524" layer="91"/>
<junction x="193.04" y="187.96"/>
<label x="195.58" y="180.34" size="1.778" layer="95"/>
<pinref part="D5" gate="G$1" pin="A"/>
<pinref part="D6" gate="G$1" pin="C"/>
</segment>
<segment>
<pinref part="R29" gate="G$1" pin="1"/>
<pinref part="R28" gate="G$1" pin="2"/>
<wire x1="243.84" y1="182.88" x2="243.84" y2="180.34" width="0.1524" layer="91"/>
<wire x1="243.84" y1="180.34" x2="243.84" y2="177.8" width="0.1524" layer="91"/>
<wire x1="243.84" y1="180.34" x2="236.22" y2="180.34" width="0.1524" layer="91"/>
<junction x="243.84" y="180.34"/>
<label x="236.22" y="180.34" size="1.778" layer="95"/>
</segment>
</net>
<net name="N$1" class="0">
<segment>
<pinref part="V1" gate="G$1" pin="VOUT"/>
<pinref part="L1" gate="G$1" pin="1"/>
<wire x1="160.02" y1="294.64" x2="162.56" y2="294.64" width="0.1524" layer="91"/>
<pinref part="C8" gate="C1" pin="1"/>
<wire x1="162.56" y1="294.64" x2="165.1" y2="294.64" width="0.1524" layer="91"/>
<wire x1="162.56" y1="294.64" x2="162.56" y2="292.1" width="0.1524" layer="91"/>
<junction x="162.56" y="294.64"/>
</segment>
</net>
<net name="N$2" class="0">
<segment>
<pinref part="V2" gate="G$1" pin="VFB"/>
<pinref part="R2" gate="G$1" pin="1"/>
<wire x1="180.34" y1="256.54" x2="182.88" y2="256.54" width="0.1524" layer="91"/>
<pinref part="R1" gate="G$1" pin="1"/>
<wire x1="182.88" y1="256.54" x2="185.42" y2="256.54" width="0.1524" layer="91"/>
<wire x1="182.88" y1="256.54" x2="182.88" y2="259.08" width="0.1524" layer="91"/>
<junction x="182.88" y="256.54"/>
</segment>
</net>
<net name="HVCC" class="0">
<segment>
<pinref part="V2" gate="G$1" pin="VOUT"/>
<pinref part="C12" gate="C1" pin="1"/>
<wire x1="180.34" y1="271.78" x2="182.88" y2="271.78" width="0.1524" layer="91"/>
<wire x1="182.88" y1="271.78" x2="198.12" y2="271.78" width="0.1524" layer="91"/>
<wire x1="198.12" y1="271.78" x2="198.12" y2="266.7" width="0.1524" layer="91"/>
<pinref part="R1" gate="G$1" pin="2"/>
<wire x1="182.88" y1="269.24" x2="182.88" y2="271.78" width="0.1524" layer="91"/>
<junction x="182.88" y="271.78"/>
<wire x1="198.12" y1="271.78" x2="213.36" y2="271.78" width="0.1524" layer="91"/>
<junction x="198.12" y="271.78"/>
<label x="208.28" y="271.78" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="LED" gate="G$1" pin="ANODE"/>
<wire x1="203.2" y1="50.8" x2="203.2" y2="53.34" width="0.1524" layer="91"/>
<wire x1="203.2" y1="53.34" x2="195.58" y2="53.34" width="0.1524" layer="91"/>
<label x="195.58" y="53.34" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="M1" gate="VCC" pin="1"/>
<wire x1="215.9" y1="134.62" x2="205.74" y2="134.62" width="0.1524" layer="91"/>
<label x="205.74" y="134.62" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="R26" gate="G$1" pin="2"/>
<wire x1="243.84" y1="294.64" x2="243.84" y2="297.18" width="0.1524" layer="91"/>
<wire x1="243.84" y1="297.18" x2="236.22" y2="297.18" width="0.1524" layer="91"/>
<label x="236.22" y="297.18" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="R32" gate="G$1" pin="2"/>
<wire x1="243.84" y1="243.84" x2="243.84" y2="246.38" width="0.1524" layer="91"/>
<wire x1="243.84" y1="246.38" x2="236.22" y2="246.38" width="0.1524" layer="91"/>
<label x="236.22" y="246.38" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="R29" gate="G$1" pin="2"/>
<wire x1="243.84" y1="193.04" x2="243.84" y2="195.58" width="0.1524" layer="91"/>
<wire x1="243.84" y1="195.58" x2="236.22" y2="195.58" width="0.1524" layer="91"/>
<label x="236.22" y="195.58" size="1.778" layer="95"/>
</segment>
<segment>
<wire x1="200.66" y1="246.38" x2="190.5" y2="246.38" width="0.1524" layer="91"/>
<label x="190.5" y="246.38" size="1.778" layer="95"/>
<pinref part="C22" gate="C1" pin="1"/>
<wire x1="200.66" y1="246.38" x2="200.66" y2="243.84" width="0.1524" layer="91"/>
<pinref part="C23" gate="C1" pin="1"/>
<wire x1="200.66" y1="246.38" x2="210.82" y2="246.38" width="0.1524" layer="91"/>
<wire x1="210.82" y1="246.38" x2="210.82" y2="243.84" width="0.1524" layer="91"/>
<junction x="200.66" y="246.38"/>
</segment>
<segment>
<pinref part="R35" gate="G$1" pin="2"/>
<wire x1="243.84" y1="109.22" x2="251.46" y2="109.22" width="0.1524" layer="91"/>
<wire x1="251.46" y1="109.22" x2="251.46" y2="106.68" width="0.1524" layer="91"/>
<label x="243.84" y="109.22" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="R37" gate="G$1" pin="2"/>
<wire x1="243.84" y1="73.66" x2="251.46" y2="73.66" width="0.1524" layer="91"/>
<wire x1="251.46" y1="73.66" x2="251.46" y2="71.12" width="0.1524" layer="91"/>
<label x="243.84" y="73.66" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="R39" gate="G$1" pin="2"/>
<wire x1="251.46" y1="35.56" x2="251.46" y2="38.1" width="0.1524" layer="91"/>
<wire x1="251.46" y1="38.1" x2="243.84" y2="38.1" width="0.1524" layer="91"/>
<label x="243.84" y="38.1" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="M2" gate="VCC" pin="1"/>
<wire x1="215.9" y1="116.84" x2="205.74" y2="116.84" width="0.1524" layer="91"/>
<label x="205.74" y="116.84" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="V1" gate="G$1" pin="VIN"/>
<wire x1="139.7" y1="294.64" x2="137.16" y2="294.64" width="0.1524" layer="91"/>
<wire x1="137.16" y1="294.64" x2="129.54" y2="294.64" width="0.1524" layer="91"/>
<wire x1="129.54" y1="294.64" x2="121.92" y2="294.64" width="0.1524" layer="91"/>
<label x="121.92" y="294.64" size="1.778" layer="95"/>
<pinref part="C7" gate="C1" pin="1"/>
<wire x1="129.54" y1="292.1" x2="129.54" y2="294.64" width="0.1524" layer="91"/>
<junction x="129.54" y="294.64"/>
<pinref part="V1" gate="G$1" pin="CE"/>
<wire x1="139.7" y1="289.56" x2="137.16" y2="289.56" width="0.1524" layer="91"/>
<wire x1="137.16" y1="289.56" x2="137.16" y2="294.64" width="0.1524" layer="91"/>
<junction x="137.16" y="294.64"/>
</segment>
</net>
<net name="N$6" class="0">
<segment>
<pinref part="V2" gate="G$1" pin="SW"/>
<pinref part="L3" gate="G$1" pin="2"/>
<wire x1="160.02" y1="261.62" x2="157.48" y2="261.62" width="0.1524" layer="91"/>
</segment>
</net>
<net name="RVCC0" class="0">
<segment>
<pinref part="R3" gate="G$1" pin="1"/>
<pinref part="C13" gate="C1" pin="1"/>
<wire x1="132.08" y1="213.36" x2="132.08" y2="210.82" width="0.1524" layer="91"/>
<wire x1="132.08" y1="210.82" x2="132.08" y2="208.28" width="0.1524" layer="91"/>
<wire x1="132.08" y1="210.82" x2="142.24" y2="210.82" width="0.1524" layer="91"/>
<junction x="132.08" y="210.82"/>
<label x="134.62" y="210.82" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="RX0" gate="G$1" pin="A"/>
<wire x1="127" y1="137.16" x2="116.84" y2="137.16" width="0.1524" layer="91"/>
<label x="116.84" y="137.16" size="1.778" layer="95"/>
</segment>
</net>
<net name="RVCC1" class="0">
<segment>
<pinref part="R4" gate="G$1" pin="1"/>
<pinref part="C14" gate="C1" pin="1"/>
<wire x1="147.32" y1="213.36" x2="147.32" y2="210.82" width="0.1524" layer="91"/>
<wire x1="147.32" y1="210.82" x2="147.32" y2="208.28" width="0.1524" layer="91"/>
<wire x1="147.32" y1="210.82" x2="157.48" y2="210.82" width="0.1524" layer="91"/>
<junction x="147.32" y="210.82"/>
<label x="149.86" y="210.82" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="RX1" gate="G$1" pin="A"/>
<wire x1="127" y1="121.92" x2="116.84" y2="121.92" width="0.1524" layer="91"/>
<label x="116.84" y="121.92" size="1.778" layer="95"/>
</segment>
</net>
<net name="RVCC2" class="0">
<segment>
<pinref part="R5" gate="G$1" pin="1"/>
<pinref part="C15" gate="C1" pin="1"/>
<wire x1="162.56" y1="213.36" x2="162.56" y2="210.82" width="0.1524" layer="91"/>
<wire x1="162.56" y1="210.82" x2="162.56" y2="208.28" width="0.1524" layer="91"/>
<wire x1="162.56" y1="210.82" x2="172.72" y2="210.82" width="0.1524" layer="91"/>
<junction x="162.56" y="210.82"/>
<label x="165.1" y="210.82" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="RX2" gate="G$1" pin="A"/>
<wire x1="127" y1="106.68" x2="116.84" y2="106.68" width="0.1524" layer="91"/>
<label x="116.84" y="106.68" size="1.778" layer="95"/>
</segment>
</net>
<net name="RVCC3" class="0">
<segment>
<pinref part="R6" gate="G$1" pin="1"/>
<pinref part="C16" gate="C1" pin="1"/>
<wire x1="177.8" y1="213.36" x2="177.8" y2="210.82" width="0.1524" layer="91"/>
<wire x1="177.8" y1="210.82" x2="177.8" y2="208.28" width="0.1524" layer="91"/>
<wire x1="177.8" y1="210.82" x2="187.96" y2="210.82" width="0.1524" layer="91"/>
<junction x="177.8" y="210.82"/>
<label x="180.34" y="210.82" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="RX3" gate="G$1" pin="A"/>
<wire x1="127" y1="91.44" x2="116.84" y2="91.44" width="0.1524" layer="91"/>
<label x="116.84" y="91.44" size="1.778" layer="95"/>
</segment>
</net>
<net name="RVCC4" class="0">
<segment>
<pinref part="R7" gate="G$1" pin="1"/>
<pinref part="C17" gate="C1" pin="1"/>
<wire x1="193.04" y1="213.36" x2="193.04" y2="210.82" width="0.1524" layer="91"/>
<wire x1="193.04" y1="210.82" x2="193.04" y2="208.28" width="0.1524" layer="91"/>
<wire x1="193.04" y1="210.82" x2="203.2" y2="210.82" width="0.1524" layer="91"/>
<junction x="193.04" y="210.82"/>
<label x="195.58" y="210.82" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="RX4" gate="G$1" pin="A"/>
<wire x1="127" y1="76.2" x2="116.84" y2="76.2" width="0.1524" layer="91"/>
<label x="116.84" y="76.2" size="1.778" layer="95"/>
</segment>
</net>
<net name="RVCC5" class="0">
<segment>
<pinref part="R8" gate="G$1" pin="1"/>
<pinref part="C18" gate="C1" pin="1"/>
<wire x1="208.28" y1="213.36" x2="208.28" y2="210.82" width="0.1524" layer="91"/>
<wire x1="208.28" y1="210.82" x2="208.28" y2="208.28" width="0.1524" layer="91"/>
<wire x1="208.28" y1="210.82" x2="218.44" y2="210.82" width="0.1524" layer="91"/>
<junction x="208.28" y="210.82"/>
<label x="210.82" y="210.82" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="RX5" gate="G$1" pin="A"/>
<wire x1="127" y1="60.96" x2="116.84" y2="60.96" width="0.1524" layer="91"/>
<label x="116.84" y="60.96" size="1.778" layer="95"/>
</segment>
</net>
<net name="N$10" class="0">
<segment>
<wire x1="40.64" y1="132.08" x2="43.18" y2="132.08" width="0.1524" layer="91"/>
<pinref part="Z1" gate="P0" pin="1"/>
<pinref part="A1" gate="A" pin="O"/>
</segment>
</net>
<net name="N$11" class="0">
<segment>
<wire x1="40.64" y1="116.84" x2="43.18" y2="116.84" width="0.1524" layer="91"/>
<pinref part="Z1" gate="P1" pin="1"/>
<pinref part="A1" gate="B" pin="O"/>
</segment>
</net>
<net name="N$12" class="0">
<segment>
<wire x1="40.64" y1="101.6" x2="43.18" y2="101.6" width="0.1524" layer="91"/>
<pinref part="Z1" gate="P2" pin="1"/>
<pinref part="A1" gate="C" pin="O"/>
</segment>
</net>
<net name="N$13" class="0">
<segment>
<wire x1="40.64" y1="81.28" x2="43.18" y2="81.28" width="0.1524" layer="91"/>
<pinref part="Z2" gate="P2" pin="1"/>
<pinref part="A2" gate="B" pin="O"/>
</segment>
</net>
<net name="N$14" class="0">
<segment>
<wire x1="40.64" y1="66.04" x2="43.18" y2="66.04" width="0.1524" layer="91"/>
<pinref part="Z2" gate="P0" pin="1"/>
<pinref part="A2" gate="C" pin="O"/>
</segment>
</net>
<net name="N$15" class="0">
<segment>
<wire x1="40.64" y1="50.8" x2="43.18" y2="50.8" width="0.1524" layer="91"/>
<pinref part="Z2" gate="P1" pin="1"/>
<pinref part="A2" gate="A" pin="O"/>
</segment>
</net>
<net name="N$3" class="0">
<segment>
<pinref part="Z1" gate="P0" pin="2"/>
<pinref part="E0" gate="G$1" pin="A"/>
<wire x1="53.34" y1="132.08" x2="55.88" y2="132.08" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$4" class="0">
<segment>
<pinref part="Z1" gate="P1" pin="2"/>
<pinref part="E1" gate="G$1" pin="A"/>
<wire x1="53.34" y1="116.84" x2="55.88" y2="116.84" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$5" class="0">
<segment>
<pinref part="Z1" gate="P2" pin="2"/>
<pinref part="E2" gate="G$1" pin="A"/>
<wire x1="53.34" y1="101.6" x2="55.88" y2="101.6" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$7" class="0">
<segment>
<pinref part="Z2" gate="P2" pin="2"/>
<pinref part="E3" gate="G$1" pin="A"/>
<wire x1="53.34" y1="81.28" x2="55.88" y2="81.28" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$8" class="0">
<segment>
<pinref part="Z2" gate="P0" pin="2"/>
<pinref part="E4" gate="G$1" pin="A"/>
<wire x1="53.34" y1="66.04" x2="55.88" y2="66.04" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$9" class="0">
<segment>
<pinref part="Z2" gate="P1" pin="2"/>
<pinref part="E5" gate="G$1" pin="A"/>
<wire x1="53.34" y1="50.8" x2="55.88" y2="50.8" width="0.1524" layer="91"/>
</segment>
</net>
<net name="M2Z" class="0">
<segment>
<pinref part="MOT2" gate="G$1" pin="2"/>
<pinref part="M1" gate="G$1" pin="OUT2"/>
<wire x1="200.66" y1="116.84" x2="185.42" y2="116.84" width="0.1524" layer="91"/>
</segment>
</net>
<net name="M3Z" class="0">
<segment>
<pinref part="MOT3" gate="G$1" pin="1"/>
<pinref part="M2" gate="G$2" pin="OUT2"/>
<wire x1="200.66" y1="99.06" x2="185.42" y2="99.06" width="0.1524" layer="91"/>
</segment>
</net>
<net name="M1Z" class="0">
<segment>
<pinref part="MOT1" gate="G$1" pin="2"/>
<pinref part="M1" gate="G$2" pin="OUT2"/>
<wire x1="200.66" y1="134.62" x2="185.42" y2="134.62" width="0.1524" layer="91"/>
</segment>
</net>
<net name="VREF_IN" class="0">
<segment>
<pinref part="R39" gate="G$1" pin="1"/>
<pinref part="R40" gate="G$1" pin="2"/>
<wire x1="251.46" y1="25.4" x2="251.46" y2="22.86" width="0.1524" layer="91"/>
<wire x1="251.46" y1="22.86" x2="251.46" y2="20.32" width="0.1524" layer="91"/>
<wire x1="251.46" y1="22.86" x2="259.08" y2="22.86" width="0.1524" layer="91"/>
<junction x="251.46" y="22.86"/>
<label x="254" y="22.86" size="1.778" layer="95"/>
<pinref part="C29" gate="C1" pin="1"/>
<wire x1="259.08" y1="22.86" x2="264.16" y2="22.86" width="0.1524" layer="91"/>
<wire x1="259.08" y1="17.78" x2="259.08" y2="22.86" width="0.1524" layer="91"/>
<junction x="259.08" y="22.86"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PB0"/>
<wire x1="88.9" y1="274.32" x2="106.68" y2="274.32" width="0.1524" layer="91"/>
<label x="93.98" y="274.32" size="1.778" layer="95"/>
</segment>
</net>
<net name="VREF_LO" class="0">
<segment>
<pinref part="R37" gate="G$1" pin="1"/>
<pinref part="R38" gate="G$1" pin="2"/>
<wire x1="251.46" y1="60.96" x2="251.46" y2="58.42" width="0.1524" layer="91"/>
<wire x1="251.46" y1="58.42" x2="251.46" y2="55.88" width="0.1524" layer="91"/>
<junction x="251.46" y="58.42"/>
<label x="254" y="58.42" size="1.778" layer="95"/>
<wire x1="251.46" y1="58.42" x2="259.08" y2="58.42" width="0.1524" layer="91"/>
<pinref part="C28" gate="C1" pin="1"/>
<wire x1="259.08" y1="58.42" x2="264.16" y2="58.42" width="0.1524" layer="91"/>
<wire x1="259.08" y1="53.34" x2="259.08" y2="58.42" width="0.1524" layer="91"/>
<junction x="259.08" y="58.42"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PA1"/>
<wire x1="88.9" y1="294.64" x2="106.68" y2="294.64" width="0.1524" layer="91"/>
<label x="96.52" y="294.64" size="1.778" layer="95"/>
</segment>
</net>
<net name="VREF_HI" class="0">
<segment>
<pinref part="R35" gate="G$1" pin="1"/>
<pinref part="R36" gate="G$1" pin="2"/>
<wire x1="251.46" y1="96.52" x2="251.46" y2="93.98" width="0.1524" layer="91"/>
<wire x1="251.46" y1="93.98" x2="251.46" y2="91.44" width="0.1524" layer="91"/>
<junction x="251.46" y="93.98"/>
<label x="254" y="93.98" size="1.778" layer="95"/>
<wire x1="251.46" y1="93.98" x2="259.08" y2="93.98" width="0.1524" layer="91"/>
<pinref part="C24" gate="C1" pin="1"/>
<wire x1="259.08" y1="93.98" x2="264.16" y2="93.98" width="0.1524" layer="91"/>
<wire x1="259.08" y1="88.9" x2="259.08" y2="93.98" width="0.1524" layer="91"/>
<junction x="259.08" y="93.98"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PA0"/>
<wire x1="88.9" y1="297.18" x2="106.68" y2="297.18" width="0.1524" layer="91"/>
<label x="96.52" y="297.18" size="1.778" layer="95"/>
</segment>
</net>
<net name="TXD1" class="0">
<segment>
<pinref part="MEM" gate="G$1" pin="SCK"/>
<wire x1="292.1" y1="289.56" x2="279.4" y2="289.56" width="0.1524" layer="91"/>
<label x="279.4" y="289.56" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PD7"/>
<wire x1="88.9" y1="210.82" x2="106.68" y2="210.82" width="0.1524" layer="91"/>
<label x="96.52" y="210.82" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="P3" gate="G$1" pin="1"/>
<wire x1="17.78" y1="170.18" x2="7.62" y2="170.18" width="0.1524" layer="91"/>
<label x="7.62" y="170.18" size="1.778" layer="95"/>
</segment>
</net>
<net name="RXD1" class="0">
<segment>
<pinref part="MEM" gate="G$1" pin="SO"/>
<wire x1="292.1" y1="292.1" x2="279.4" y2="292.1" width="0.1524" layer="91"/>
<label x="279.4" y="292.1" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PD6"/>
<wire x1="88.9" y1="213.36" x2="106.68" y2="213.36" width="0.1524" layer="91"/>
<label x="96.52" y="213.36" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="P3" gate="G$1" pin="2"/>
<wire x1="17.78" y1="167.64" x2="7.62" y2="167.64" width="0.1524" layer="91"/>
<label x="7.62" y="167.64" size="1.778" layer="95"/>
</segment>
</net>
<net name="XCKD1" class="0">
<segment>
<pinref part="MEM" gate="G$1" pin="SI"/>
<wire x1="292.1" y1="294.64" x2="279.4" y2="294.64" width="0.1524" layer="91"/>
<label x="279.4" y="294.64" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PD5"/>
<wire x1="88.9" y1="215.9" x2="96.52" y2="215.9" width="0.1524" layer="91"/>
<label x="91.44" y="215.9" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="P3" gate="G$1" pin="3"/>
<wire x1="17.78" y1="165.1" x2="7.62" y2="165.1" width="0.1524" layer="91"/>
<label x="7.62" y="165.1" size="1.778" layer="95"/>
</segment>
</net>
<net name="!SS" class="0">
<segment>
<pinref part="MEM" gate="G$1" pin="!CS"/>
<wire x1="292.1" y1="297.18" x2="279.4" y2="297.18" width="0.1524" layer="91"/>
<label x="279.4" y="297.18" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PR1"/>
<wire x1="22.86" y1="223.52" x2="7.62" y2="223.52" width="0.1524" layer="91"/>
<label x="7.62" y="223.52" size="1.778" layer="95"/>
</segment>
</net>
<net name="RGB_R" class="0">
<segment>
<pinref part="F2" gate="G$1" pin="G"/>
<wire x1="218.44" y1="10.16" x2="210.82" y2="10.16" width="0.1524" layer="91"/>
<label x="210.82" y="10.16" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PE5"/>
<wire x1="88.9" y1="193.04" x2="104.14" y2="193.04" width="0.1524" layer="91"/>
<label x="96.52" y="193.04" size="1.778" layer="95"/>
</segment>
</net>
<net name="RGB_G" class="0">
<segment>
<pinref part="UC" gate="G$1" pin="PE4"/>
<wire x1="88.9" y1="195.58" x2="104.14" y2="195.58" width="0.1524" layer="91"/>
<label x="96.52" y="195.58" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="F1" gate="G$2" pin="G"/>
<wire x1="198.12" y1="10.16" x2="190.5" y2="10.16" width="0.1524" layer="91"/>
<label x="190.5" y="10.16" size="1.778" layer="95"/>
</segment>
</net>
<net name="RGB_B" class="0">
<segment>
<pinref part="F1" gate="G$1" pin="G"/>
<wire x1="177.8" y1="10.16" x2="170.18" y2="10.16" width="0.1524" layer="91"/>
<label x="170.18" y="10.16" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="UC" gate="G$1" pin="PD4"/>
<wire x1="88.9" y1="218.44" x2="104.14" y2="218.44" width="0.1524" layer="91"/>
<label x="96.52" y="218.44" size="1.778" layer="95"/>
</segment>
</net>
<net name="N$16" class="0">
<segment>
<pinref part="F1" gate="G$1" pin="D"/>
<pinref part="R17" gate="G$1" pin="1"/>
<wire x1="182.88" y1="17.78" x2="182.88" y2="20.32" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$17" class="0">
<segment>
<pinref part="F1" gate="G$2" pin="D"/>
<pinref part="R16" gate="G$1" pin="1"/>
<wire x1="203.2" y1="17.78" x2="203.2" y2="20.32" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$18" class="0">
<segment>
<pinref part="F2" gate="G$1" pin="D"/>
<pinref part="R15" gate="G$1" pin="1"/>
<wire x1="223.52" y1="17.78" x2="223.52" y2="20.32" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$19" class="0">
<segment>
<pinref part="R17" gate="G$1" pin="2"/>
<wire x1="182.88" y1="30.48" x2="182.88" y2="33.02" width="0.1524" layer="91"/>
<wire x1="182.88" y1="33.02" x2="195.58" y2="33.02" width="0.1524" layer="91"/>
<pinref part="LED" gate="G$1" pin="B"/>
<wire x1="195.58" y1="33.02" x2="195.58" y2="35.56" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$20" class="0">
<segment>
<pinref part="R16" gate="G$1" pin="2"/>
<pinref part="LED" gate="G$1" pin="G"/>
<wire x1="203.2" y1="30.48" x2="203.2" y2="35.56" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$25" class="0">
<segment>
<pinref part="R15" gate="G$1" pin="2"/>
<wire x1="223.52" y1="30.48" x2="223.52" y2="33.02" width="0.1524" layer="91"/>
<wire x1="223.52" y1="33.02" x2="210.82" y2="33.02" width="0.1524" layer="91"/>
<pinref part="LED" gate="G$1" pin="R"/>
<wire x1="210.82" y1="33.02" x2="210.82" y2="35.56" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$21" class="0">
<segment>
<pinref part="M1" gate="G$2" pin="OUT1"/>
<pinref part="R41" gate="G$1" pin="2"/>
<wire x1="185.42" y1="144.78" x2="187.96" y2="144.78" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$22" class="0">
<segment>
<pinref part="R41" gate="G$1" pin="1"/>
<pinref part="MOT1" gate="G$1" pin="1"/>
<wire x1="198.12" y1="144.78" x2="200.66" y2="144.78" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$23" class="0">
<segment>
<pinref part="M1" gate="G$1" pin="OUT1"/>
<pinref part="R42" gate="G$1" pin="2"/>
<wire x1="185.42" y1="127" x2="187.96" y2="127" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$24" class="0">
<segment>
<pinref part="R42" gate="G$1" pin="1"/>
<pinref part="MOT2" gate="G$1" pin="1"/>
<wire x1="198.12" y1="127" x2="200.66" y2="127" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$26" class="0">
<segment>
<pinref part="M2" gate="G$2" pin="OUT1"/>
<pinref part="R43" gate="G$1" pin="2"/>
<wire x1="185.42" y1="109.22" x2="187.96" y2="109.22" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$27" class="0">
<segment>
<pinref part="R43" gate="G$1" pin="1"/>
<pinref part="MOT3" gate="G$1" pin="2"/>
<wire x1="198.12" y1="109.22" x2="200.66" y2="109.22" width="0.1524" layer="91"/>
</segment>
</net>
</nets>
</sheet>
</sheets>
</schematic>
</drawing>
</eagle>
