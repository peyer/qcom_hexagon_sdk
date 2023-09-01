#!/usr/bin/env python
#------------------------------------------------------------------------------
# Copyright (c) 2014-2017 Qualcomm Technologies, Inc.  All Rights Reserved
#------------------------------------------------------------------------------
from __future__ import absolute_import

import sys
try:
    import objdump
except ImportError:
    from profiler import objdump

try:
    import stathist
except ImportError:
    from profiler import stathist
import os

MAX_CONTRIB_ITEMS = 200
MIN_CONTRIB_PCT = 0
MAX_CUM_PCT = 100
DETAIL_FRAC = 1


boilerplate_start = r"""
<!DOCTYPE HTML>
<!-- saved from url=(0025)https://www.qualcomm.com/ -->
<html>
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
  <title>{0}</title>"""

boilerplate_start2 = r"""
  <link rel="stylesheet" href="https://code.jquery.com/ui/1.11.4/themes/smoothness/jquery-ui.css">
  <style>
    body { font-size:12px; }
    .hlA { background-color: yellow; }
    .ulA { text-decoration:underline; }
    .jt { cursor:pointer; }
    .jtf { color:dimgray;font-style:italic }
    .rightpane_select, .pc_scroll { text-decoration:none; cursor:pointer; }
    .rightpane_select:hover, .pc_scroll:hover, .jt:hover { text-decoration:underline; color:darkred; }
    .slick-header.ui-state-default,.slick-headerrow.ui-state-default{width:100%;overflow:hidden;border-left:0}
    .slick-header-columns,.slick-headerrow-columns{position:relative;white-space:nowrap;cursor:default;overflow:hidden}
    .slick-header-column.ui-state-default{position:relative;display:inline-block;overflow:hidden;-o-text-overflow:ellipsis;text-overflow:ellipsis;margin:0;padding:4px;border-right:1px solid silver;border-left:0;border-top:0;border-bottom:0;float:left}
    .slick-headerrow-column.ui-state-default{padding:4px}
    .slick-header-column-sorted{font-style:italic}
    .slick-sort-indicator{display:inline-block;width:8px;height:5px;margin-left:4px;margin-top:6px;float:left}
    .slick-resizable-handle{position:absolute;font-size:.1px;display:block;cursor:col-resize;width:4px;right:0;top:0;height:100%}
    .slick-sortable-placeholder{background:silver}
    .grid-canvas{position:relative;outline:1; border-left:1px solid silver}
    .slick-row.ui-state-active,.slick-row.ui-widget-content{position:absolute;border:0;width:100%}
    .slick-cell,.slick-headerrow-column{position:absolute;border:1px solid transparent;border-right:1px dotted silver;border-bottom-color:silver;overflow:hidden;-o-text-overflow:ellipsis;text-overflow:ellipsis;vertical-align:middle;z-index:1;padding:1px 2px 2px 1px;margin:0;white-space:nowrap;cursor:default}
    .slick-group-toggle{display:inline-block}
    .slick-cell.highlighted{background:#87cefa;background:rgba(0,0,255,.2);-webkit-transition:all .5s;-moz-transition:all .5s;-o-transition:all .5s;transition:all .5s}
    .slick-cell.flashing{border:1px solid red!important}
    .slick-cell.editable{z-index:11;overflow:visible;background:#fff;border-color:#000;border-style:solid}
    .slick-cell:focus{outline:0}.slick-reorder-proxy{display:inline-block;background:#00f;opacity:.15;filter:alpha(opacity=15);cursor:move}.slick-reorder-guide{display:inline-block;height:2px;background:#00f;opacity:.7;filter:alpha(opacity=70)}.slick-selection{z-index:10;position:absolute;border:2px dashed #000}
    .ui-helper-hidden { display:none; }
    .ui-helper-hidden-accessible { position:absolute !important; clip:rect(1px 1px 1px 1px); clip:rect(1px,1px,1px,1px); }
    .ui-helper-reset { margin:0; padding:0; border:0; outline:0; line-height:1.3; text-decoration:none; font-size:1em; list-style:none; }
    .ui-helper-clearfix:after { content:"."; display:block; height:0; clear:both; visibility:hidden; }
    .ui-helper-clearfix { display:inline-block; }
    * html .ui-helper-clearfix { height:1%; }
    .ui-helper-clearfix { display:block; }
    .ui-helper-zfix { width:100%; height:100%; top:0; left:0; position:absolute; opacity:0; filter:Alpha(Opacity=0); }
    .ui-state-disabled { cursor:default !important; }
    .ui-icon { display:block; text-indent:-99999px; overflow:hidden; background-repeat:no-repeat; }
    .ui-widget-overlay { position:absolute; top:0; left:0; width:100%; height:100%; }
    .ui-widget { font-family:Verdana,Arial,sans-serif; }
    .ui-widget .ui-widget { font-size:1em; }
    .ui-widget input, .ui-widget select, .ui-widget textarea, .ui-widget button { font-family:Verdana,Arial,sans-serif; font-size:1em; }
    .ui-widget-content { border:1px solid #aaaaaa; background:#ffffff; color:#222222; }
    .ui-widget-content a { color:#222222; }
    .ui-widget-header { border:1px solid #aaaaaa; background:#cccccc; color:#222222; font-weight:bold; }
    .ui-widget-header a { color:#222222; }
    .ui-state-default, .ui-widget-content .ui-state-default, .ui-widget-header .ui-state-default { border:1px solid #d3d3d3; background:#e6e6e6; font-weight:normal; color:#555555; }
    .ui-state-default a, .ui-state-default a:link, .ui-state-default a:visited { color:#555555; text-decoration:none; }
    .ui-state-hover, .ui-widget-content .ui-state-hover, .ui-widget-header .ui-state-hover, .ui-state-focus, .ui-widget-content .ui-state-focus, .ui-widget-header .ui-state-focus { border:1px solid #999999; background:#dadada; font-weight:normal; color:#212121; }
    .ui-state-hover a, .ui-state-hover a:hover { color:#212121; text-decoration:none; }
    .ui-state-active, .ui-widget-content .ui-state-active, .ui-widget-header .ui-state-active { border:1px solid #aaaaaa; background:#ffffff; font-weight:normal; color:#212121; }
    .ui-state-active a, .ui-state-active a:link, .ui-state-active a:visited { color:#212121; text-decoration:none; }
    .ui-widget :active { outline:none; }
    .ui-state-highlight, .ui-widget-content .ui-state-highlight, .ui-widget-header .ui-state-highlight  {border:1px solid #fcefa1; background:#fbf9ee; color:#363636; }
    .ui-state-highlight a, .ui-widget-content .ui-state-highlight a,.ui-widget-header .ui-state-highlight a { color:#363636; }
    .ui-state-error, .ui-widget-content .ui-state-error, .ui-widget-header .ui-state-error {border:1px solid #cd0a0a; background:#fef1ec; color:#cd0a0a; }
    .ui-state-error a, .ui-widget-content .ui-state-error a, .ui-widget-header .ui-state-error a { color:#cd0a0a; }
    .ui-state-error-text, .ui-widget-content .ui-state-error-text, .ui-widget-header .ui-state-error-text { color:#cd0a0a; }
    .ui-priority-primary, .ui-widget-content .ui-priority-primary, .ui-widget-header .ui-priority-primary { font-weight:bold; }
    .ui-priority-secondary, .ui-widget-content .ui-priority-secondary,  .ui-widget-header .ui-priority-secondary { opacity:.7; filter:Alpha(Opacity=70); font-weight:normal; }
    .ui-state-disabled, .ui-widget-content .ui-state-disabled, .ui-widget-header .ui-state-disabled { opacity:.35; filter:Alpha(Opacity=35); background-image:none; }
    .ui-corner-all, .ui-corner-top, .ui-corner-left, .ui-corner-tl { -moz-border-radius-topleft:4px; -webkit-border-top-left-radius:4px; -khtml-border-top-left-radius:4px; border-top-left-radius:4px; }
    .ui-corner-all, .ui-corner-top, .ui-corner-right, .ui-corner-tr { -moz-border-radius-topright:4px; -webkit-border-top-right-radius:4px; -khtml-border-top-right-radius:4px; border-top-right-radius:4px; }
    .ui-corner-all, .ui-corner-bottom, .ui-corner-left, .ui-corner-bl { -moz-border-radius-bottomleft:4px; -webkit-border-bottom-left-radius:4px; -khtml-border-bottom-left-radius:4px; border-bottom-left-radius:4px; }
    .ui-corner-all, .ui-corner-bottom, .ui-corner-right, .ui-corner-br { -moz-border-radius-bottomright:4px; -webkit-border-bottom-right-radius:4px; -khtml-border-bottom-right-radius:4px; border-bottom-right-radius:4px; }
    .ui-widget-overlay { background:#aaaaaa; opacity:.30;filter:Alpha(Opacity=30); }
    .ui-widget-shadow { margin:-8px 0 0 -8px; padding:8px; background:#aaaaaa; opacity:.30;filter:Alpha(Opacity=30); -moz-border-radius:8px; -khtml-border-radius:8px; -webkit-border-radius:8px; border-radius:8px; }
    .ui-tabs .ui-tabs-panel { padding: 0 0.5em 0 0.5em }

    * {
        -webkit-box-sizing:border-box;
           -moz-box-sizing:border-box;
                box-sizing:border-box;
    }

    #tabs {
        position:relative;
    }

    .proctab {
        position:relative;
        top:0;
        left:0;
        height:600px;
    }

    .tophalf {
        height:35%;
    }

    .leftpane{
        width:50%;
        max-height:100%;
        overflow-y:auto;
        display:block;
        float:left;
    }

    .rightpane{
        width:50%;
        max-height:100%;
        overflow-y:auto;
        display:block;
        float:right;
    }

    .myGrid {
        display:inline-block;
        width:100%;
        height:65%;
    }

    #lefttable {
        width:100%;
       height:100%;
    }

    .a_nopad {
        text-align:right;
    }

    .a_rpad {
        text-align:right;
        padding-right:10px;
    }

    .a_lpad {
        text-align:right;
        padding-left:10px;
    }

    .a_rlpad {
        text-align:right;
        padding-left:5px;
        padding-right:5px;
    }

    .lpad {
        padding-left:10px;
    }

    .rlpad {
        padding-left:5px;
        padding-right:5px;
    }

    .lg_hdr {
        font-size:1.15em;
        font-weight:bold;
        vertical-align:bottom;
        margin-top:1em;
    }

    .sm_hdr {
        font-size:0.85em;
        font-weight:bold;
        vertical-align:bottom;
        text-align:right;
    }

    .sm_hdr_lpad {
        font-size:0.85em;
        font-weight:bold;
        vertical-align:bottom;
        text-align:center;
        padding-left:10px;
    }

    #dragbar{
       background-color:black;
       height:700px;
       float:right;
       width:3px;
       opacity:.2;
       cursor:col-resize;
    }
    #ghostbar{
        width:3px;
        background-color:#000;
        opacity:0.2;
        position:absolute;
        cursor:col-resize;
        z-index:999}
    #vertbar{
       background-color:black;
       height:3px;
       width:100%;
       position:absolute;
       bottom:0;
       opacity:.2;
       cursor:row-resize;
    }
    #ghostvert{
        width:100%;
        height:3px:
        background-color:#000;
        opacity:0.2;
        position:absolute;
        cursor:row-resize;
        z-index:999}

    tr .bold {
        font-weight:bold;
        background-color:lightblue;
    }

    #stats table {
        border-collapse:collapse;
    }

    #stats th {
        border:1px solid grey;
        background:lightgrey;
    }

    #stats td {
        border:1px solid grey;
        padding:3px;
        margin:0;
    }

    #derived_stats table {
        border-collapse:collapse;
    }

    #derived_stats th {
        border:1px solid grey;
        background:lightgrey;
    }

    #derived_stats td {
        border:1px solid grey;
        padding:3px;
        margin:0;
    }

    #ihist table {
        border-collapse:collapse;
    }

    #ihist th {
        border:1px solid grey;
        background:lightgrey;
    }

    #ihist td {
        border:1px solid grey;
        padding:3px;
        margin:0;
    }

    .hamburger {
        display: inline-block;
        cursor: pointer;
        margin-top: 6px;
        margin-left: 10px;
        margin-right: 10px;
    }

    .hbBar {
        width: 20px;
        height: 3px;
        background-color: dimgray;
        margin-top: 2px;
    }

    .sidenav {
        height: 100%;
        position: fixed;
        z-index: 1000;
        top: 0;
        left: -260px;
        width: 250px;
        background-color: #f1f1f1;
        box-shadow: 8px 0px 8px gray;
        overflow-x: hidden;
        -webkit-transition: all 0.5s;
        -moz-transition:    all 0.5s;
        transition:         all 0.5s;
        padding: 5px;
        font-family: Verdana,Arial,sans-serif;
        font-size: 1em;
    }

    .sidenav a {
        text-decoration: none;
        font-size: 1em;
        color: gray;
        display: block;
        -webkit-transition: all 0.5s;
        -moz-transition:    all 0.5s;
        transition:         all 0.5s
    }

    .sidenav a:hover{
        color: black;
    }

    .sidenav .closeButton {
        float: right;
        font-size: 1.25em;
        cursor: pointer;
        border-width: 1px;
        border-style: solid;
        border-color: #f1f1f1;
    }

    .sidenav .closeButton:hover {
        border-color: black;
    }

    .sidenav .dockButton {
        float: right;
        font-size: 1.25em;
        cursor: pointer;
        border-width: 1px;
        border-style: solid;
        border-color: #f1f1f1;
        color: gray;
    }

    .sidenav .dockButton:hover {
        border-color: black;
    }

    @media screen and (max-height: 450px) {
        .sidenav {
            padding: 3px;
        }

        .sidenav a {
            font-size: 0.85em;
        }
    }

    hr {
        height: 1px;
        border: 0;
        background-color: lightgray;
        color: lightgray;
    }

    fieldset {
        border: 1px solid silver;
    }

    .switch {
        position: relative;
        display: inline-block;
        width: 32px;
        height: 18px;
        vertical-align: -5px;
    }

    .switch input {display:none;}

    .slider {
        position: absolute;
        cursor: pointer;
        top: 0;
        left: 0;
        right: 0;
        bottom: 0;
        background-color: #ccc;
        -webkit-transition: all 0.2s;
        -moz-transition:    all 0.2s;
        transition:         all 0.2s;
    }

    .slider:before {
        position: absolute;
        content: "";
        height: 14px;
        width: 14px;
        left: 2px;
        bottom: 2px;
        background-color: white;
        -webkit-transition: all 0.2s;
        -moz-transition:    all 0.2s;
        transition:         all 0.2s;
    }

    .slider:hover {
        box-shadow: 2px 2px 2px gray;
    }

    input:checked + .slider {
        background-color: dodgerblue;
    }

    input:checked + .slider:before {
        -webkit-transform: translateX(14px);
        -moz-transform:    translateX(14px);
        transform:         translateX(14px);
    }

    /* Rounded sliders */
    .slider.round {
        border-radius: 18px;
    }

    .slider.round:before {
        border-radius: 50%;
    }

    .p303 {
        padding: 3px 0 3px 0;
    }

    .p30320 {
        padding: 3px 0 3px 20px;
    }

    input[type=search] {
        width: 250px;
        border-radius: 4px;
        border-top-right-radius: 0;
        border-bottom-right-radius: 0;
        border-bottom-left-radius: 0;
        border: 1px solid #ccc;
        border-bottom: none;
        border-right: none;
        font-size: 1em;
        background-color: #e6e6e6;
        color: black;
        padding: 6.5px 0px 7.5px 10px;
        value: "";
        -webkit-transition: all 0.2s;
        transition: all 0.2s;
    }

    input[type=search]:focus {
        background-color: white;
    }

    input[type=search]:focus + .clearSearch {
        background-color: white;
        cursor: pointer;
    }

    .clearSearch {
        border: 1px solid #ccc;
        border-left: 0;
        border-bottom: 0;
        border-radius: 4px;
        border-top-left-radius: 0;
        border-bottom-left-radius: 0;
        background-color: #e6e6e6;
        padding: 6.5px 5px 7.5px 5px;
        color: gray;
    }
  </style>

  <!-- <link rel="stylesheet" href="examples.css" type="text/css"/> -->
</head>
<body style="background-color:#f1f1f1;">
  <div id="tabs">
"""

# A list goes here, as such:
# <ul>
#   <li><a href="#CORE">CORE</a></li>
#   <li><a href="#HVX">HVX</a></li>
#   <li><a href="#stats">stats</a></li>
#   <li><a href="#events">events</a></li>
# </ul>


# NOTE: This is a format string with 1 argument (the processor name). Any '{' or '}' characters must be doubled.
boilerplate_topleft = """
    <div id="{0}" class="proctab">
      <div id="{0}_tophalf" class="tophalf">
        <div id="{0}_leftpane" class="leftpane">
"""


boilerplate_topleft_events = """
    <div id="{0}" class="proctab">
      <div id="{0}_tophalf" class="tophalf">
        <div id="{0}_leftpane" class="leftpane">
"""


# stalls and bus events go here
boilerplate_topbetween = """
          <!-- <div id="dragbar"></div> -->
        </div>

        <div id="{0}_rightpane" class="rightpane">"""



# top pcs/functions go here
boilerplate_topright = """        </div> <!-- end rightpane -->
      </div> <!-- end tophalf -->

      <div id="{0}_myGrid" class="myGrid">
      </div>

    </div> <!-- end {0} -->
"""



boilerplate_mid = """
  </div> <!-- end tabs -->

  <script src="https://code.jquery.com/jquery-1.7.2.min.js"></script>
  <script src="https://code.jquery.com/ui/1.11.4/jquery-ui.js"></script>

  <script>{0}</script>

  <script>
    var showLeadingZeros =  {1};
    var show_0x          = '{2}';
    var showPacketBraces =  {3};
    var showJumpToFuncs  =  {4};
    var fixChromeMagnify =  {5};
    var dockMenu         =  {6};
    var underlineJumps   =  {7};
    var highlightJumps   =  {8};
    var showSelectedType =  {9};
    var i = 0;
    var dragging = false;
    var CurrentHashTabName;
    var CurrentTabName;
    var CurrentTab;
"""


boilerplate_mid2 = """
    var Grid = {};
    var myTopRow_id = {};
    var myTopRow = {};
    var tgtStack = {};
    var selectedTypePerTab = {};

    // horizontal resize

/*
    $('#dragbar').mousedown(function(e)
    {
        e.preventDefault();
        dragging = true;
        var rightpane = $('#rightpane');
        var ghostbar = $('<div>', { id:'ghostbar',
                                    css: { height:rightpane.outerHeight(), top:rightpane.offset().top, left:rightpane.offset().left }

                       }).appendTo('body');

        $(document).mousemove(function(e) {
            ghostbar.css("left",e.pageX+2);
        });
    });

    $(document).mouseup(function(e) {
        if (dragging) {
            $('#leftpane').css("width",e.pageX+2);
            $('#rightpane').css("left",e.pageX+2);
            $('#ghostbar').remove();
            $(document).unbind('mousemove');
            dragging = false;
        }
    });
*/

    // vertical resize

/*
    var draggingV = 0;

    $('#vertbar').mousedown(function(e) {
        e.preventDefault();
        draggingV = true;
        var bottomhalf = $('#myGrid');
        var ghostvert = $('<div>', { id:'ghostvert',
                                     css: { // width:bottomhalf.outerWidth(),
                                                 top:bottomhalf.offset().top,
                                            //  left:bottomhalf.offset().left
                                          }
                                   }).appendTo('body');

        $(document).mousemove(function(e) {
            ghostvert.css("top",e.pageY+2);
        });
    });

    $(document).mouseup(function(e) {
        if (draggingV) {
            console.log(e.pageY);
            $('#tophalf').css("bottom",e.pageY+2);
            $('#myGrid').css("top",e.pageY+2);
            $('#ghostvert').remove();
            $(document).unbind('mousemove');
            draggingV = false;
        }
    });
*/

    var options = {
        enableCellNavigation:       true,
        enableTextSelectionOnCells: true,
        enableColumnReorder:        true,
        forceFitColumns:            false,
        fullWidthRows:              true,
        rowHeight:                  20,
        syncColumnCellResize:       true
    };
    var aggregate = '';
    var showCommits = false;
    var showStalls = false;
    var showMajor = false;
    var showMinor = false;
    var showEvents = false;
    var showHLL = true;
    var selectedType = '';

    var browserZoom = document.body.style.zoom;
    var browserDPR  = window.devicePixelRatio;
    var browserName = getBrowserName();

    function getBrowserName() {
        var str = navigator.userAgent;

        if (str.indexOf('Chrome') != -1) {
            if (browserDPR > 1) {
                if (fixChromeMagnify)
                    document.body.style.zoom = 1.0 / browserDPR;
            }
            else {
               fixChromeMagnify = false;
            }
            return 'Chrome';
        }

        fixChromeMagnify = false;

        if (str.indexOf('Firefox') != -1) {
            return 'Firefox';
        }
        else if (!!document.documentMode) {
            return 'IE';
        }
        else if (!!window.StyleMedia) {
            return 'Edge';
        }
        else if (str.indexOf('Safari') != -1) {
            return 'Safari';
        }
        return 'unknown';
    }

    function isStallMinor(item) {
        // item is a Slick Grid row
        return item['p'] < 1; // DETAIL_FRAC
    }

    function hasLowPotential(item) {
        // item is a Slick Grid row
        return item['r'] < 50 || item['d'] == "TOFF_CYCLES";
    }

    if (typeof String.prototype.endsWith !== 'function') {
        String.prototype.endsWith = function(suf) {
            return this.slice(-suf.length) === suf;
        }
    }

    $(function () {
        $('#tabs').tabs({
            beforeActivate: function(event, ui) {
                var NewHashTabName = ui.newTab.children().attr('href')
                var newTab = NewHashTabName.replace(/^#/,'');
                var curTab = CurrentTabName;

                if ((curTab == 'CORE') || (curTab == 'HVX') || (curTab == 'events')) {
                    SaveButtonState (curTab);
                }

                if (newTab == 'CORE') {
                    if (curTab != 'HVX') {
                        $('#DX1').show();
                        $('#DX2').show();
                        $('#DX3').show();
                        $('#DX4').show();

                        if (curTab != 'events') {
                            $('#FX1').show();
                            $('#searchContainer').show();
                        }
                    }
                    RestoreButtonState (newTab);
                }
                else if (newTab == 'HVX') {
                    if (curTab != 'CORE') {
                        $('#DX1').show();
                        $('#DX2').show();
                        $('#DX3').show();
                        $('#DX4').show();

                        if (curTab != 'events') {
                            $('#FX1').show();
                            $('#searchContainer').show();
                        }
                    }
                    RestoreButtonState (newTab);
                }
                else if (newTab == 'events') {
                    if ((curTab == 'CORE') || (curTab == 'HVX')) {
                        $('#DX1').hide();
                        $('#DX2').hide();
                        $('#DX3').hide();
                        $('#DX4').hide();
                    }
                    else {
                        $('#FX1').show();
                        $('#searchContainer').show();
                    }
                    RestoreButtonState (newTab);
                }
                else {
                    $('#searchContainer').hide();
                    $('#FX1').hide();
                }

                CurrentHashTabName = NewHashTabName;
                CurrentTabName     = newTab;
                CurrentTab         = $(NewHashTabName);
                selectedType       = selectedTypePerTab[newTab];
            }
        });

        var tabsWithButtons = ['CORE', 'HVX', 'events'];

        var b1 = {};
        var b2 = {};
        var b3 = {};
        var b4 = {};
        var b5 = {};
        var b6 = {};
        var b7 = {};
        var t1 = {};
        var t2 = {};
        var t3 = {};
        var t4 = {};
        var t5 = {};
        var t6 = {};
        var t7 = {};

        function SaveButtonState(tab) {
            b1[tab] = IX1.checked;
            b2[tab] = IX2.checked;
            b3[tab] = IX3.checked;
            b4[tab] = IX4.checked;
            b5[tab] = IX5.checked;
            b6[tab] = IX6.checked;
            b7[tab] = IX7.checked;

            t1[tab] = TX1.title;
            t2[tab] = TX2.title;
            t3[tab] = TX3.title;
            t4[tab] = TX4.title;
            t5[tab] = TX5.title;
            t6[tab] = TX6.title;
            t7[tab] = TX7.title;
        }

        function RestoreButtonState(tab) {
            IX1.checked  = b1[tab];
            IX2.checked  = b2[tab];
            IX3.checked  = b3[tab];
            IX4.checked  = b4[tab];
            IX5.checked  = b5[tab];
            IX6.checked  = b6[tab];
            IX7.checked  = b7[tab];

            TX1.title = t1[tab];
            TX2.title = t2[tab];
            TX3.title = t3[tab];
            TX4.title = t4[tab];
            TX5.title = t5[tab];
            TX6.title = t6[tab];
            TX7.title = t7[tab];

            showCommits      = IX1.checked;
            showStalls       = IX2.checked;
            showMajor        = IX3.checked;
            showMinor        = IX4.checked;
            showEvents       = IX5.checked;
            showHLL          = IX6.checked;
            showSelectedType = IX7.checked;
        }

        function InitButtonState() {
            var count = tabsWithButtons.length;
            var tab;
            var i;

            for (i = 0; i < count; i++) {
                tab = tabsWithButtons[i];

                b1[tab] = IX1.checked;
                b2[tab] = IX2.checked;
                b3[tab] = IX3.checked;
                b4[tab] = IX4.checked;
                b5[tab] = IX5.checked;
                b6[tab] = IX6.checked;
                b7[tab] = IX7.checked;

                t1[tab] = TX1.title;
                t2[tab] = TX2.title;
                t3[tab] = TX3.title;
                t4[tab] = TX4.title;
                t5[tab] = TX5.title;
                t6[tab] = TX6.title;
                t7[tab] = TX7.title;
            }
        }

        InitButtonState();

        CurrentHashTabName = $('li.ui-tabs-active').children().attr('href');
        CurrentTabName     = CurrentHashTabName.replace(/^#/,'');
        CurrentTab         = $(CurrentHashTabName);

        if ((browserName == 'Chrome') && (browserDPR > 1)) {
            if (fixChromeMagnify) {
                IX12.checked = true;
                TX12.title = TX12.title.replace('override','revert back to');
            }
        }
        else {
            $('#FX2').hide();
        }

        if (browserName != 'Firefox') {
            $('#clearSearch1').hide();
        }

        if (dockMenu) {
            openNav();
            dockNav();
        }

        document.getElementById('search1').oninput = function(){ searchProftool() };

        const myColor = {
            'commit':       '#007000',
            'stallMajor':   '#b00000',
            'stallMinor':   'lightcoral',
            'dim':          'dimgray',
            'hll':          'gray',
            'event':        'mediumblue',
            'bg_highlight': 'lightblue',
            'default':      'black'
        }

        var highlight_pc = 1;
        var first_repane = true;

        var rightpane_select_jq = $(".rightpane_select");

        rightpane_select_jq.map(function () {
            if ($(this).data("target").endsWith("rightpane_cycles")) {
                $(this).addClass("bold");
                which_right_pane_select = this;
                repane($(this).data('target'));
            }
            else if ($(this).data("target").endsWith("events_rightpane_L2_ACCESS")) {
                selectedTypePerTab['events'] = 'L2_ACCESS';
                $(this).addClass("bold");
                if (first_repane == false) {
                    repane($(this).data('target'));
                }
                else {
                    var pane = $(this).data('target');
                    CurrentTab.data('which_right_pane', pane);
                    $("#" + pane + aggregate).show();
                    first_repane = false;
                }
            }
        });

        function repane(pane) {
            if (pane.indexOf(CurrentTabName) == 0) {
                CurrentTab.find('.rightpane').find('div').hide();
            }
            CurrentTab.data('which_right_pane', pane);
            $("#" + pane + aggregate).show();
        }

        function refresh_grid() {
            var tab = CurrentTabName;
            var dv = DV[tab];
            var grid=Grid[tab];
            var pc_to_idx = PC_to_IDX[tab];
            var id = myTopRow_id[tab];
            var item = dv.getItemById(id);

            selectedTypePerTab[tab] = selectedType;

            dv.refresh();

            if (tab != 'events') {
                if (showHLL) {
                    while (!myNewFilter(item) && (id > 0)) {
                        id -= 1;
                        item = dv.getItemById(id);
                    }

                    if (id > 0) {
                        var prevItem = dv.getItemById(id-1);

                        if ((item['l'] == 1) && (item['f'] != 1) && (prevItem['l'] == 1) && (prevItem['f'] == 1)) {
                            id -= 1;
                        }
                    }
                }
                else {
                    if ((item['l'] == 1) && (item['f'] == 1)) {
                        // Top row is a filename.  Go to the next one down.
                        do {
                            id += 1;
                            item = dv.getItemById(id);
                        }
                        while (!myNewFilter(item));
                    }
                    while (!myNewFilter(item) && (id > 0)) {
                        id -= 1;
                        item = dv.getItemById(id);
                    }
                }
            }
            else {
                if (showHLL) {
                    while (!ev_myNewFilter(item) && (id > 0)) {
                        id -= 1;
                        item = dv.getItemById(id);
                    }

                    if (id > 0) {
                        var prevItem = dv.getItemById(id-1);

                        if ((item['l'] == 1) && (item['f'] != 1) && (prevItem['l'] == 1) && (prevItem['f'] == 1)) {
                            id -= 1;
                        }
                    }
                }
                else {
                    if ((item['l'] == 1) && (item['f'] == 1)) {
                        // Top row is a filename.  Go to the next one down.
                        do {
                            id += 1;
                            item = dv.getItemById(id);
                        }
                        while (!ev_myNewFilter(item));
                    }
                    while (!ev_myNewFilter(item) && (id > 0)) {
                        id -= 1;
                        item = dv.getItemById(id);
                    }
                }
            }

            myTopRow_id[tab] = id;

            var topRow = dv.getRowById(myTopRow_id[tab])
            grid.invalidate(topRow);
            grid.scrollRowToTop(topRow);
        }

        rightpane_select_jq.click(function() {
            var tgt = $(this).data('target');

            const key = '_rightpane_';
            selectedType = tgt.slice(tgt.indexOf(key) + key.length);

            if ((selectedType == 'cycles') || (selectedType == 'stall_total') || (selectedType == 'commits')) {
                selectedType = '';
            }

            // make the old link (if any) not bold
            CurrentTab.find('.rightpane_select').removeClass('bold');

            // make the new link bold
            $(this).addClass("bold");

            repane(tgt);
            refresh_grid();
        });

        var gridwidth = $('.myGrid').width();


        function fn_formatter(row,cell,value,columnDef,dataContext) {
            if (dataContext['l'] == 1) {
                if (value > 1) {
                    var addr = dataContext['p'];
                    var fn = FuncNames[value][0];
                    var offset = addr - FuncNames[value][1];
                    var tcolor;

                    if (offset == 0) {
                        tcolor = myColor['default'] + '; font-weight:bold';
                    }
                    else {
                        tcolor = myColor['dim'] + '; font-style:italic';
                        fn = fn + '+' + offset.toString(16);
                    }

                    return '<span style="color:' + tcolor + '" title="' + fn + '">' + fn + '</span>';
                }
            }
            return '';
        }


        function pct_formatter(row,cell,value,columnDef,dataContext) {
            var level = dataContext['l'];
            var tcolor;
            var rpad;

            if (level == 2) {
                // This row contains a stall/commit count

                tcolor = myColor[(dataContext['d'] == 'commits') ? 'commit' : 'stallMajor'];
                rpad = '20';
            }
            else if (level == 3) {
                // This row contains stall details

                tcolor = myColor[isStallMinor(dataContext) ? 'stallMinor' : 'stallMajor'];
                rpad = '0';
            }
            else {
                return '';
            }

            return '<span style="float:right; color:' + tcolor + '; padding-right:' + rpad + 'px;" title="' + value + '%">'  + value + '%</span>';
        }


        function addr_formatter(row,cell,value,columnDef,dataContext) {
            if (dataContext['l'] == 1) {
                // This row contains an address
                if (dataContext['f'] == 1) {
                    return '';
                }

                var tcolor = myColor['default'];

                if (highlight_pc === value)
                    tcolor = tcolor + '; background-color:' + myColor['bg_highlight'];

                addr = value.toString(16);

                if (showLeadingZeros)
                    addr = ('00000000' + addr).slice(-8);

                return '<span style="color:' + tcolor + '">' + show_0x + addr + '</span>';
            }
            return '';
        }


        function stall_formatter(row,cell,value,columnDef,dataContext) {
            var level = dataContext['l'];
            var myfmt; // formatting for bar (background color, float direction)

            if (level == 1)
            {
                // This row contains an address
                return '';
            }

            if (dataContext['d'] == 'commits')
                myfmt = 'float:left; background:' + myColor['commit'];

            else if (isStallMinor(dataContext))
                myfmt = 'float:right; background:' + myColor['stallMinor'];

            else
                myfmt = 'float:right; background:' + myColor['stallMajor'];

            return '<span style="height:6px; display:inline-block;' + myfmt + '; width:' + value + '%"></span>';
        }


        function count_formatter(row,cell,value,columnDef,dataContext) {
            var level = dataContext['l'];
            var tcolor;
            var rpad;

            if (level == 1) {
                // This row contains an address

                if (dataContext['f'] == 1) {
                    // This row contains an hll address
                    return '';
                }

                tcolor = myColor['default'];
                rpad = '20';
            }
            else if (level == 2) {
                // This row contains a stall/commit count

                tcolor = myColor[(dataContext['d'] == 'commits') ? 'commit' : 'stallMajor'];
                rpad = '20';
            }
            else if (level == 3) {
                // This row contains stall details

                tcolor = myColor[isStallMinor(dataContext) ? 'stallMinor' : 'stallMajor'];
                rpad = '0';
            }
            else if (level == 5) {
                tcolor = myColor['event'];
                rpad = '0';
            }
            else {
                return '';
            }

            return '<span style="float:right; color:' + tcolor + '; padding-right:' + rpad + 'px;" title="' + value + '">'  + value + "</span>";
        }


        function ev_count_formatter(row,cell,value,columnDef,dataContext) {
            if (dataContext['l'] == 5) {
                return  '<span style="float:right; color:' + myColor['event'] + '" title="' + value + '">'  + value + '</span>';
            }
            return '';
        }


        function AddLinks (value, curAddr) {
            var tab = CurrentTabName;
            var pc_to_idx;
            var dv = DV[tab];
            var myClasses = JumpToClasses();

            var w = value.split (' ');
            var i;
            var len = w.length;
            var str;

            if (tab == 'events')
                pc_to_idx = PC_to_IDX['CORE'];
            else
                pc_to_idx = PC_to_IDX[tab];

            for (i = 0; i < len; i++) {
                str = w[i];
                if ((str.indexOf(':') >= 4) || ((str == 'jump') || (str == 'call')) || (str.length == 5) && (str[4] == 'r')) {
                    i += 1;
                    if (i < len) {
                        str = w[i];
                        if (str[0] == '0') {
                            var target = parseInt(str,16).toString(16);
                            var index = pc_to_idx[target];
                            var replacement;
                            var target = '0x' + target;

                            if (index == undefined) {
                                var msg = 'Packet at address ' + target + ' was not executed during the test run.';
                                replacement = '<a class="' + myClasses + '" onclick="alert(\\\'' + msg + '\\\')">' + target + '</a>';
                            }
                            else {
                                var row = dv.getRowById(index);
                                if (row == undefined) {
                                    if (showHLL == false)
                                        row = dv.getRowById(index+1);

                                    if (row == undefined) {
                                        console.log ("AddLinks: row=undefined index=%d curAddr=0x%s target=0x%s value=%s", row, index, curAddr.toString(16), target.toString916, value);
                                        return value;
                                    }
                                }
                                replacement = '<a class="' + myClasses + '" onclick="JumpToRow('+row.toString()+','+index+','+ curAddr +')">' + target + '</a>';
                            }
                            w[i] = str.replace(target, replacement);
                        }
                    }
                }
            }

            value = w.join(' ');
            return value;

            function JumpToClasses() {
                var classes = "jt";
                if (underlineJumps)
                    classes += " ulA";
                if (highlightJumps)
                    classes += " hlA";
                return classes;
            }
        }


        function disasm_formatter(row,cell,value,columnDef,dataContext) {
            var level = dataContext['l'];
            var tcolor = myColor['default'];
            var lpad = '40';
            var title = value;

            if (level == 1) {
                // This row contains an address
                lpad = '0';

                if (dataContext['f'] == 1) {
                    // This row contains an hll address

                    var hll_filenum = dataContext['p'];

                    if (hll_filenum > 0) {
                        var hll_filename = FileNames[hll_filenum];
                        var hll_linenum = dataContext['c'];
                        value = hll_filename + ':' + hll_linenum
                        tcolor = myColor['hll'];
                        title = value;
                    }
                }
                else {
                    var mapObj = {
                        '&':'&amp;',
                        '<=':'&lt;=',
                        '>=':'&gt;=',
                        '<<':'&lt;&lt;',
                        '>>':'&gt;&gt;',
                        '<':'<span class="jtf">',
                        '>':'</span>'
                    }
                    title = title.replace(/;/g, ';\\n');

                    if (!showJumpToFuncs)
                        value = value.replace(/\s*[<][a-z0-9\.\+_]*[>]\s*/gi, '');

                    value = value.replace(/&|<=|>=|<<|>>|<|>/g, function (matched){return mapObj[matched];});

                    value = AddLinks(value, dataContext['p']);

                    if (showPacketBraces) {
                        value = '{ ' + value + ' }'
                    }
                }
            }
            else if (level == 2) {
                // This row contains a stall/commit count

                if (value == 'commits') {
                    tcolor = myColor['commit'];
                    value = 'Commits';
                }
                else {
                    tcolor = myColor['stallMajor'];

                    if (value == 'stall_total')
                        value = 'Stalls';
                }

                lpad = '20';
            }
            else if (level == 3) {
                // This row contains stall details

                tcolor = myColor[isStallMinor(dataContext) ? 'stallMinor' : 'stallMajor'];

                if (hasLowPotential(dataContext)) {
                    tcolor = tcolor + '; font-style:italic';
                    title = title + '\\nLow stall-reduction potential';
                }
            }
            else if (level == 5) {
                tcolor = myColor['event'];
                lpad = '40';
                title = value;
            }

            return  '<span style="color:' + tcolor + '; padding-left:' + lpad + 'px" title="' + title + '">'  + value + "</span>";
        }


        function ev_disasm_formatter(row,cell,value,columnDef,dataContext) {
            var level = dataContext['l'];
            var tcolor = 'black';
            var lpad = '0';
            var title = value;

            if (level == 1) {
                // This row contains an address

                if (dataContext['f'] == 1) {
                    // This row contains an hll address

                    var hll_filenum = dataContext['p'];

                    if (hll_filenum > 0) {
                        var hll_filename = FileNames[hll_filenum];
                        var hll_linenum = dataContext['c'];
                        value = hll_filename + ':' + hll_linenum;
                        tcolor = 'gray';
                    }
                }
                else {
                    var mapObj = {
                        '&':'&amp;',
                        '<=':'&lt;=',
                        '>=':'&gt;=',
                        '<<':'&lt;&lt;',
                        '>>':'&gt;&gt;',
                        '<':'<span class="jtf">',
                        '>':'</span>'
                    }
                    title = title.replace(/;/g, ';\\n');

                    if (!showJumpToFuncs)
                        value = value.replace(/\s*[<][a-z0-9\.\+_]*[>]\s*/gi, '');

                    value = value.replace(/&|<=|>=|<<|>>|<|>/g, function (matched){return mapObj[matched];});

                    value = AddLinks(value, dataContext['p']);

                    if (showPacketBraces) {
                        value = '{ ' + value + ' }'
                    }
                }
            }
            else if (level == 5) {
                tcolor = myColor['event'];
                lpad = '40';
            }

            return '<span style="color:' + tcolor + '; padding-left:' + lpad + 'px" title="' + title + '">'  + value + '</span>';
        }


        function myNewFilter(item) {
            level = item['l'];

            if (level == 1) {
                if (item['f'] == 1) {
                    return showHLL;
                }
                // Asm on this line
                return true;
            }
            else if (level == 2) {
                if (item['d'] == 'commits') {
                    return showCommits;
                }
                return showStalls;
            }
            else if (level == 3) {
                if (!showSelectedType || ((selectedType == '') || (selectedType == item['d']))) {
                    if (isStallMinor(item)) {
                        return showMinor;
                    }
                    return showMajor;
                }
            }
            else if (level == 5) {
                if (!showSelectedType || ((selectedType == '') || (selectedType == item['d']))) {
                    return showEvents;
                }
            }
            return false;
        }

        function ev_myNewFilter(item) {
            level = item['l'];

            if (level == 1) {
                if (item['f'] == 1) {
                    return showHLL;
                }
                // Asm on this line
                return true;
            }
            else if (level == 5) {
                if (!showSelectedType || ((selectedType == '') || (selectedType == item['d']))) {
                    return showEvents;
                }
            }
            return false;
        }

        var fr_p20 = "<span style='float:right;padding-right:20px'>"
        var fr     = "<span style='float:right'>"

        var columns = [
            {id: "fn",     name: "<b>Function</b>",                 field: "f", width: (180), formatter:     fn_formatter, resizable: true},
            {id: "addr",   name: "<b>Address</b>",                  field: "p", width: ( 90), formatter:   addr_formatter, resizable: true},
            {id: "stall",  name: "<b><center>Stalls</center></b>",  field: "p", width: (100), formatter:  stall_formatter, resizable: true},
            {id: "pct",    name: fr_p20 + "<b>Percent</b></span>",  field: "p", width: ( 85), formatter:    pct_formatter, resizable: true},
            {id: "count",  name: fr_p20 + "<b>Count</b></span>",    field: "c", width: (100), formatter:  count_formatter, resizable: true},
            {id: "disasm", name: "<b>Disassembly / Stall Name</b>", field: "d", width: (500), formatter: disasm_formatter, resizable: true}
        ];

        var ev_columns = [
            {id: "fn"  ,   name: "<b>Function</b>",                 field: "f", width: (180), formatter:        fn_formatter, resizable: true},
            {id: "addr",   name: "<b>Address</b>",                  field: "p", width: ( 90), formatter:      addr_formatter, resizable: true},
            {id: "count",  name: fr + "<b>Event Count</b></span>",  field: "c", width: (100), formatter:  ev_count_formatter, resizable: true},
            {id: "disasm", name: "<b>Disassembly / Event Name</b>", field: "d", width: (500), formatter: ev_disasm_formatter, resizable: true}
        ];


        const data = {};
        const PC_to_IDX = {};
        var DV = {};

"""

# data.[...] = [ ..., ... ] goes here

boilerplate_mid3 = """
        var i;

        // for each processor, make a grid
        for (processor in data) {

            if (data.hasOwnProperty(processor)) {

                var dv = new Slick.Data.DataView({ inlineFilters: true});
                var grid = new Slick.Grid('#'+processor+'_myGrid', dv, columns, options);
                var pc_to_idx = PC_to_IDX[processor];

                Grid[processor] = grid;
                DV[processor] = dv;
                myTopRow_id[processor] = 0;
                myTopRow[processor] = 0;

                grid.onScroll.subscribe(function (e,args) {
                    var tab = CurrentTabName;

                    if ((tab == 'CORE') || (tab == 'HVX')) {
                        var topPixel = args.scrollTop;
                        var topRow = ~~(topPixel / options.rowHeight);

                        if (topRow != myTopRow[tab]) {
                            var dv = DV[tab];
                            myTopRow_id[tab] = dv.getItem(topRow).id;
                        }
                    }
                });

                dv.onRowCountChanged.subscribe(function (e, args) {
                    grid.updateRowCount();
                    grid.render();
                });

                dv.onRowsChanged.subscribe(function(e,args) {
                    grid.invalidateRows(args.rows);
                    grid.render();
                });

                dv.beginUpdate();
                dv.setItems(data[processor]);
                dv.setFilter(myNewFilter);
                dv.endUpdate();

                setupResizing(gridwidth,grid,processor);

                tgtStack[processor] = [];

                $('#' + processor).data('which_right_pane', processor + '_rightpane_cycles');

                if (processor == 'CORE') {
                    $("input[type=checkbox][name=disasm_chkbx]").change(function(e) {
                        var thisInput = $(e.target);
                        var checked = thisInput.is(':checked');
                        var value = thisInput.val();
                        var thisSpan   = document.getElementById('bx' + value);
                        var thisButton = document.getElementById('TX' + value);
                        var tab = CurrentTabName;
                        var grid = Grid[tab];
                        var dv = DV[tab];

                        if (checked) {
                            thisButton.title = thisButton.title.replace('show','hide');
                            thisButton.title = thisButton.title.replace('override','revert back to');
                        }
                        else {
                            thisButton.title = thisButton.title.replace('hide','show');
                            thisButton.title = thisButton.title.replace('revert back to','override');
                        }

                        if (value == 1)
                            showCommits = checked;
                        else if (value == 2)
                            showStalls = checked;
                        else if (value == 3) {
                            showMajor = checked;
                            if (showMajor) {
                                TX3.title = 'Click to hide:\\nStall details';
                                TX4.title = 'Click to show:\\nMinor Stalls';
                            }
                            else {
                                TX3.title = 'Click to show:\\nStall details';
                                TX4.title = 'Click to show:\\nStall details + Minor stalls';

                                if (showMinor) {
                                    showMinor = false;
                                    IX4.checked = false;
                                }
                            }
                        }
                        else if (value == 4) {
                            showMinor = checked;
                            if (showMinor) {
                                if (!showMajor) {
                                    showMajor = true;
                                    IX3.checked = true;
                                }
                                TX3.title = 'Click to hide:\\nStall details + Minor stalls';
                                TX4.title = 'Click to hide:\\nMinor stalls';
                            }
                            else if (showMajor) {
                                TX3.title = 'Click to hide:\\nStall details';
                                TX4.title = 'Click to show:\\nMinor stalls';
                            }
                        }
                        else if (value == 5)
                            showEvents = checked;
                        else if (value == 6)
                            showHLL = checked;
                        else if (value == 7)
                            showSelectedType = checked;
                        else if (value == 8)
                            showJumpToFuncs = checked;
                        else if (value == 9)
                            showPacketBraces = checked;
                        else if (value == 10)
                            showLeadingZeros = checked;
                        else if (value == 11) {
                            if (checked)
                                show_0x = '0x';
                            else
                                show_0x = '';
                        }
                        else if (value == 12) {
                            fixChromeMagnify = checked;

                            if (fixChromeMagnify)
                                document.body.style.zoom = 1.0 / browserDPR;
                            else
                                document.body.style.zoom = browserZoom;

                            setupResizing(gridwidth,grid,tab);
                        }
                        else if (value == 14) {
                            underlineJumps = checked;
                        }
                        else if (value == 15) {
                            highlightJumps = checked;
                        }

                        dv.refresh();

                        var id = myTopRow_id[tab];
                        var item = dv.getItemById(id);

                        if (tab != 'events') {
                            if (showHLL) {
                                while (!myNewFilter(item) && (id > 0)) {
                                    id -= 1;
                                    item = dv.getItemById(id);
                                }

                                if (id > 0) {
                                    var prevItem = dv.getItemById(id-1);

                                    if ((item['l'] == 1) && (item['f'] != 1) && (prevItem['l'] == 1) && (prevItem['f'] == 1)) {
                                        id -= 1;
                                    }
                                }
                            }
                            else {
                                if ((item['l'] == 1) && (item['f'] == 1)) {
                                    // Top row is a filename.  Go to the next one down.
                                    do {
                                        id += 1;
                                        item = dv.getItemById(id);
                                    }
                                    while (!myNewFilter(item));
                                }
                                while (!myNewFilter(item) && (id > 0)) {
                                    id -= 1;
                                    item = dv.getItemById(id);
                                }
                            }
                        }
                        else {
                            if (showHLL) {
                                while (!ev_myNewFilter(item) && (id > 0)) {
                                    id -= 1;
                                    item = dv.getItemById(id);
                                }

                                if (id > 0) {
                                    var prevItem = dv.getItemById(id-1);

                                    if ((item['l'] == 1) && (item['f'] != 1) && (prevItem['l'] == 1) && (prevItem['f'] == 1)) {
                                        id -= 1;
                                    }
                                }
                            }
                            else {
                                if ((item['l'] == 1) && (item['f'] == 1)) {
                                    // Top row is a filename.  Go to the next one down.
                                    do {
                                        id += 1;
                                        item = dv.getItemById(id);
                                    }
                                    while (!ev_myNewFilter(item));
                                }
                                while (!ev_myNewFilter(item) && (id > 0)) {
                                    id -= 1;
                                    item = dv.getItemById(id);
                                }
                            }
                        }

                        myTopRow_id[tab] = id;

                        var topRow = dv.getRowById(myTopRow_id[tab])
                        grid.invalidate(topRow);
                        grid.scrollRowToTop(topRow);
                    }.bind(undefined));
                }

                $("input[type=radio][name="+processor+"_funcag]").change(function(e) {
                    var val = parseInt($(e.target).val())
                    if (val == 2) {
                        aggregate = "_fa";
                    }
                    else {
                        aggregate = "";
                    }
                    repane(CurrentTab.data('which_right_pane'));
                });

                var targetpc_str = '';
                var targetpc_idx = 0;

                var win3_grid = function(pc_to_idx, tgt_str, grid, dv) {
                    targetpc_str = tgt_str;
                    targetpc_idx = pc_to_idx[tgt_str];
                    if (targetpc_idx == undefined) return;
                    highlight_pc = parseInt(targetpc_str,16);

                    var item = dv.getItemById(targetpc_idx);
                    while (!myNewFilter(item)) {
                        targetpc_idx += 1;
                        item = dv.getItemById(targetpc_idx);
                    }

                    var row = dv.getRowById(targetpc_idx);
                    tgtStack[CurrentTabName].push(targetpc_str);
                    grid.invalidate(row);
                    grid.scrollRowToTop(row);
                }.bind(this, pc_to_idx);

                $('#' + processor).find(".pc_scroll").click(function(grid, dv, win3_grid, e) {
                    $(".pc_scroll").css('background-color','#ffffff');
                    $(e.target).css('background-color',myColor['bg_highlight']);
                    targetpc_str = $(e.target).attr("data-target");
                    win3_grid (targetpc_str, grid, dv);
                }.bind(this, grid, dv, win3_grid));

                $(".ui-tabs-anchor").click(function(e) {
                    $(window).resize();
                })
            }
        }

        // make an event grid
        {
            processor = "CORE";
            {
                var pc_to_idx = PC_to_IDX[processor];

                var ev_dv = new Slick.Data.DataView({ inlineFilters: true});
                var ev_grid = new Slick.Grid("#events_myGrid", ev_dv, ev_columns, options);

                Grid['events'] = ev_grid;
                DV['events'] = ev_dv;
                myTopRow_id['events'] = 0;
                myTopRow['events'] = 0;

                ev_grid.onScroll.subscribe(function (e,args) {
                    var tab = CurrentTabName;

                    if (tab == 'events') {
                        var topPixel = args.scrollTop;
                        var topRow = ~~(topPixel / options.rowHeight);

                        if (topRow != myTopRow[tab]) {
                            var dv = DV[tab];
                            myTopRow_id[tab] = dv.getItem(topRow).id;
                        }
                    }
                });

                ev_dv.onRowCountChanged.subscribe(function (e, args) {
                    ev_grid.updateRowCount();
                    ev_grid.render();
                });

                ev_dv.onRowsChanged.subscribe(function(e,args) {
                    ev_grid.invalidateRows(args.rows);
                    ev_grid.render();
                });

                ev_dv.beginUpdate();
                ev_dv.setItems(data[processor]);
                ev_dv.setFilter(ev_myNewFilter);
                ev_dv.endUpdate();

                setupResizing(gridwidth,ev_grid,processor);

                tgtStack['events'] = [];

                $('#events').data('which_right_pane', 'events_rightpane_L2_ACCESS');

                $("input[type=radio][name=ev_funcag]").change(function(e) {
                    var val = parseInt($(e.target).val())
                    if (val == 2) {
                        aggregate = "_fa";
                    }
                    else {
                        aggregate = "";
                    }
                    repane(CurrentTab.data('which_right_pane'));
                });

                var targetpc_str = '';
                var targetpc_idx = 0;

                var ev_win3_grid = function(pc_to_idx, tgt_str, ev_grid, ev_dv) {
                    targetpc_str = tgt_str;
                    targetpc_idx = pc_to_idx[tgt_str];
                    if (targetpc_idx == undefined) return;
                    highlight_pc = parseInt(targetpc_str,16);

                    var item = ev_dv.getItemById(targetpc_idx);
                    while (!ev_myNewFilter(item)) {
                        targetpc_idx += 1;
                        item = ev_dv.getItemById(targetpc_idx);
                    }

                    var row = ev_dv.getRowById(targetpc_idx);
                    tgtStack['events'].push(targetpc_str);
                    ev_grid.invalidate(row);
                    ev_grid.scrollRowToTop(row);
                }.bind(this, pc_to_idx);

                $("#events").find(".pc_scroll").click(function(ev_grid, ev_dv, ev_win3_grid, e) {
                    $(".pc_scroll").css('background-color','#ffffff');
                    $(e.target).css('background-color',myColor['bg_highlight']);
                    targetpc_str = $(e.target).attr("data-target");
                    ev_win3_grid (targetpc_str, ev_grid, ev_dv);
                }.bind(this, ev_grid, ev_dv, ev_win3_grid));
            }
        }

        function getSelectionText() {
            var text = "";
            if (window.getSelection) {
                text = window.getSelection().toString();
            }
            else if (document.selection && document.selection.type != "Control") {
                text = document.selection.createRange().text;
            }
            return text;
        }

        $(document).ready(function () {
            $('div').mouseup(function (e) {
                var seld = getSelectionText();
                seld = seld.replace('0x','');
                if (seld.length > 0) {
                    win3_grid(seld);
                }
            })
        });

        function scrollToIndex(index) {
            var tab = CurrentTabName;
            var grid = Grid[tab];
            var dv = DV[tab];
            var row;

            var item = dv.getItemById(index);

            if (!showHLL) {
                if ((item['l'] == 1) && (item['f'] == 1)) {
                    index += 1;
                    item = dv.getItemById(index);
                }
            }

            while (!myNewFilter(item) && (index > 0)) {
                index -= 1;
                item = dv.getItemById(index);
            }

            myTopRow_id[tab] = index;
            row = dv.getRowById(index);

            grid.invalidate(row);
            grid.scrollRowToTop(row);
        }

        $(document).keydown(function(e) {
            if (e.keyCode == 27) {
                var tab = CurrentTabName;
                var pc_to_idx;
                var tgt_stack = tgtStack[tab];

                if (tab == 'events')
                    pc_to_idx = PC_to_IDX['CORE'];
                else
                    pc_to_idx = PC_to_IDX[tab];

                if (tgt_stack.length >= 1) {
                    targetpc_str = tgt_stack.pop();

                    index = pc_to_idx[parseInt(targetpc_str,16).toString(16)];

                    if (index == undefined)
                        return;
                    scrollToIndex(index);
                }
            }
        });

        function searchForFunc(pc_to_idx,value) {
            var i;
            for (i in FuncNames) {
                if (FuncNames[i][0].indexOf(value) == 0) {
                    var index = pc_to_idx[FuncNames[i][1].toString(16)];
                    if (index != undefined) {
                        return index;
                        break;
                    }
                }
            }
            return -1;
        }

        function searchProftool() {
            var value = $("#search1").val();
            if (value == '') {
                return;
            }
            var tab = CurrentTabName;

            if (tab == 'events')
                pc_to_idx = PC_to_IDX['CORE'];
            else
                pc_to_idx = PC_to_IDX[tab];

            var index = searchForFunc(pc_to_idx,value);

            if (index < 0) {
                index = pc_to_idx[parseInt(value,16).toString(16)];
                if (index == undefined)
                    return;
            }
            scrollToIndex(index);
        }
    });

    function setupResizing(gridwidth,grid,processor) {
        resizeTabs(gridwidth,grid,processor);
        $(window).resize(resizeTabs.bind(this,gridwidth,grid,processor));

        function resizeTabs(gridwidth,grid,processor) {
            var newHeight = window.innerHeight - 60; // subtract to correct for margins, padding, etc.

            if (fixChromeMagnify)
                newHeight = (browserDPR * newHeight) + 14;

            $('.proctab').css({height:newHeight});

            var cols = grid.getColumns();

            if (cols.length > 4) {
                gridwidth = $('#'+processor+'_myGrid').width();
                cols[5].width = gridwidth - cols[0].width - cols[1].width - cols[2].width - cols[3].width - cols[4].width - 21;
            }
            else {
                gridwidth = $('#events_myGrid').width();
                cols[3].width = gridwidth - cols[0].width - cols[1].width - cols[2].width - 21;
            }
            grid.setColumns(cols);
            grid.resizeCanvas();
        }
    }

    // Support for menu hamburger

    function hbFocus() {
        var focusColor = "black";
        hbBar1.style["backgroundColor"] = focusColor;
        hbBar2.style["backgroundColor"] = focusColor;
        hbBar3.style["backgroundColor"] = focusColor;
    }

    function hbFade() {
        var fadeColor = "dimgray";
        hbBar1.style["backgroundColor"] = fadeColor;
        hbBar2.style["backgroundColor"] = fadeColor;
        hbBar3.style["backgroundColor"] = fadeColor;
    }

    // Support for side navigation menu

    function openNav() {
        sideNav1.style.left = "0px";
    }

    function closeNav() {
        sideNav1.style["boxShadow"]    = "8px 0px 8px gray";
        sideNav1.style.left            = "-260px";
        tabs.style.marginLeft          = "0";
        $("#dockButton1").show();
        $("#hamburger1").show();
        tabs.style["webkitTransition"] = "all 0";
        tabs.style["mozTransition"]    = "all 0";
        tabs.style.transition          = "all 0";
    }

    function dockNav() {
        tabs.style["webkitTransition"] = "all 0.5s";
        tabs.style["mozTransition"]    = "all 0.5s";
        tabs.style.transition          = "all 0.5s";
        tabs.style.marginLeft          = "250px";
        $("#dockButton1").hide();
        $("#hamburger1").hide();
        sideNav1.style["boxShadow"]    = "none";
    }

    $(document).ready(function(){
        $("#clearSearch1").click(function(){
            $("#search1").focus();
            $("#search1").val("");
        });
    });

    function JumpToRow(row,index,pc) {
        Grid[CurrentTabName].invalidate(row);
        Grid[CurrentTabName].scrollRowToTop(row);
        myTopRow_id[CurrentTabName] = index;
        myTopRow[CurrentTabName] = row;
        tgtStack[CurrentTabName].push(pc.toString(16));
    }
  </script>
"""

boilerplate_end = """
  <div id="sideNav1" class="sidenav">
    <div>
      <span style="color:#555; font-size:1.25em; float:left;"><b>Hexagon Profiler</b></span>
      <span><a class="dockButton  ui-corner-all" onclick="dockNav()"  title="Dock Menu" id="dockButton1" style="margin-left:10px">&gt;&gt;</a></span>
      <span><a class="closeButton ui-corner-all" onclick="closeNav()" title="Close Menu">&lt;&lt;</a></span>
    </div>
    <div>
      <div style="color:gray; font-size:0.8em; float:left; clear:right"><b>{0}</b></div>
      <p><br><br><hr><br>
      <fieldset id="FX1">
        <legend style="color:#555"><b> Disassembly Panel </b></legend>
        <fieldset style="margin:0.5em" id="FX1A">
          <legend style="color:#555"><b>Visible Lines</b></legend>
          <div class="p303" id="DX1"><label class="switch"><input type="checkbox" name="disasm_chkbx" id='IX1' value="1"        ><div class="slider round" id="TX1" title="Click to show:\nCommit counts"></div></label><span id="bx1"> Commits</span></div>
          <div class="p303" id="DX2"><label class="switch"><input type="checkbox" name="disasm_chkbx" id='IX2' value="2"        ><div class="slider round" id="TX2" title="Click to show:\nStall counts" ></div></label><span id="bx2"> Stalls</span></div>
          <div class="p303" id="DX3"><label class="switch"><input type="checkbox" name="disasm_chkbx" id='IX3' value="3"        ><div class="slider round" id="TX3" title="Click to show:\nStall details"></div></label><span id="bx3"> Stall Details</span></div>
          <div class="p30320"
                            id="DX4"><label class="switch"><input type="checkbox" name="disasm_chkbx" id='IX4' value="4"        ><div class="slider round" id="TX4" title="Click to show:\nStall details + Minor stalls"></div></label><span id="bx4"> Minor Stalls</span></div>
          <div class="p303" id="DX5"><label class="switch"><input type="checkbox" name="disasm_chkbx" id='IX5' value="5"        ><div class="slider round" id="TX5" title="Click to show:\nEvents"></div></label><span id="bx5"> Events</span></div>
          <div class="p303" id="DX6"><label class="switch"><input type="checkbox" name="disasm_chkbx" id='IX6' value="6" checked><div class="slider round" id="TX6" title="Click to hide:\nSource filenames"></div></label><span id="bx6"> Filenames</span></div>
          <div class="p303" id="DX7"><label class="switch"><input type="checkbox" name="disasm_chkbx" id='IX7' value="7"        ><div class="slider round" id="TX7" title="Click to show:\nSelected Type Only"></div></label><span id="bx7"> Selected Type Only</span></div>
        </fieldset>
        <fieldset style="margin:2em 0.5em 0.5em 0.5em" id="FX1B">
          <legend style="color:#555"><b> Visible Attributes </b></legend>
          <div class="p303" id="DX8"> <label class="switch"><input type="checkbox" name="disasm_chkbx" id='IX8'  value="8"  {1} ><div class="slider round" id="TX8"  title="Click to {2}:\nFunction names for targets of jumps & calls"></div></label><span id="bx8" > Jump Target Names</span></div>
          <div class="p303" id="DX9"> <label class="switch"><input type="checkbox" name="disasm_chkbx" id='IX9'  value="9"  {3} ><div class="slider round" id="TX9"  title="Click to {4}:\nBraces around packets">         </div></label><span id="bx9" > Packet Braces</span></div>
          <div class="p303" id="DX10"><label class="switch"><input type="checkbox" name="disasm_chkbx" id='IX10' value="10" {5} ><div class="slider round" id="TX10" title="Click to {6}:\nLeading zeros in addresses">    </div></label><span id="bx10"> Leading Zeros</span></div>
          <div class="p303" id="DX11"><label class="switch"><input type="checkbox" name="disasm_chkbx" id='IX11' value="11" {7} ><div class="slider round" id="TX11" title="Click to {8}:\nLeading 0x in addresses">       </div></label><span id="bx11"> Hex Prefix 0x</span></div>
          <div class="p303" id="DX14"><label class="switch"><input type="checkbox" name="disasm_chkbx" id='IX14' value="14" {9} ><div class="slider round" id="TX14" title="Click to {10}:\nUnderline address links">      </div></label><span id="bx14"> Underline links</span></div>
          <div class="p303" id="DX15"><label class="switch"><input type="checkbox" name="disasm_chkbx" id='IX15' value="15" {11}><div class="slider round" id="TX15" title="Click to {12}:\nHighlight address links">      </div></label><span id="bx15"> Highlight links</span></div>
        </fieldset>
      </fieldset>
      <br><br>
      <fieldset id="FX2" style="padding-right:0">
        <legend style="color:#555"><b> Global Options </b></legend>
        <div class="p303" id="DX12"><label class="switch"><input type="checkbox" name="disasm_chkbx" id='IX12' value="12"><div class="slider round" id="TX12" title="Click to override:\nMagnification change introduced in Chrome 54"></div></label><span id="bx12"> Override Chrome54+ magnifier</span></div>
      </fieldset>
    </div>
  </div>
</body>
</html>
"""



def htquote(mystr):
    return mystr.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;").replace("\n","<br>")


def myColor(name):
    if (name == 'cycles'):      return 'mediumblue'
    if (name == 'commits'):     return '#007000'
    if (name == 'stall_total'): return '#b00000'
    if (name == 'error'):       return '#b00000'
    return ''


# Emit per-stall top consumers by PC
def gen_stallinfo(f,name,stalltotal,pc_to_func,stats,is_fa,name_to_pc,name_to_func, processor):
    if is_fa:
        # functions
        fastr     = "_fa"
        which     = "functions"
        addr_hdr  = "StartAddr"
        which_hdr = "Function"
    else:
        # packets
        fastr     = ""
        which     = "packets"
        addr_hdr  = "Address"
        which_hdr = "Packet"

    pctop = sorted (sorted (stats.items(), key=lambda x: x[0]), key=lambda x: x[1], reverse=True)

    f.write ('          <div id="%s_rightpane_%s%s" style="display:none;">\n' % (processor, name, fastr))

    displayname = name

    if name == "cycles":
        displayname = "Commits and Stalls"
    elif name == "stall_total":
        displayname = "Stalls"
    elif name == "commits":
        displayname = "Commits"

    f.write ('            <table>\n')
    f.write ('              <tr>\n')
    f.write ('                <th colspan=4 style="padding-top:10px;">%s</th>\n' % displayname)
    f.write ('                <th></th>\n')
    f.write ('              </tr>\n')
    f.write ('              <tr>\n')
    f.write ('                <td class="sm_hdr">#</td>\n')
    f.write ('                <td class="sm_hdr_lpad">Count</td>\n')
    f.write ('                <td class="sm_hdr_lpad">Pct</td>\n')
    f.write ('                <td class="sm_hdr_lpad">CumPct</td>\n')
    f.write ('                <td class="sm_hdr_lpad">%s</td>\n' % addr_hdr)
    f.write ('                <td class="sm_hdr_lpad" style="text-align:left;">%s</td>\n' % which_hdr)
    f.write ('              </tr>\n')

    stallpct = 0
    cumpct = 0
    cum = 0
    n_items = 0

    global showLeadingZeros
    global s0x

    for name,stalls in pctop:
        if (stalls > 0):
            n_items += 1
            if (n_items <= MAX_CONTRIB_ITEMS):
                pc   = name_to_pc(name,pc_to_func)
                func = name_to_func(name,pc_to_func)
                cum += stalls

                stallpct = 100.0 * stalls/stalltotal
                cumpct   = 100.0 * cum/stalltotal

                if isinstance(pc, int):
                    f.write ('<tr>')
                    f.write (  '<td class="a_nopad">%d</td>' % n_items)
                    f.write (  '<td class="a_lpad">%d</td>' % stalls)
                    f.write (  '<td class="a_lpad">%02.1f%%</td>' % stallpct)
                    f.write (  '<td class="a_lpad">%02.1f%%</td>' % cumpct)

                    f.write (  '<td class="lpad"><span class="pc_scroll" data-target="%x">%s' % (pc, s0x))
                    if (showLeadingZeros): f.write ('%08x' % pc)
                    else:                  f.write ('%x'   % pc)
                    f.write ('</span></td>')

                    f.write (  '<td class="lpad">%s' % htquote(func))
                    if not is_fa:
                        start_addr = objdump.symtab.get(func, 0)
                        if (start_addr > 0): f.write ('<i> +%s%x</i>' % (s0x, pc - start_addr))
                    f.write (  '</td>')
                    f.write ('</tr>\n')

                else:
                    f.write ('<tr>')
                    f.write (  '<td class="a_nopad">%d</td>' % n_items)
                    f.write (  '<td class="a_lpad">%d</td>' % stalls)
                    f.write (  '<td class="a_lpad">%02.1f%%</td>' % stallpct)
                    f.write (  '<td class="a_lpad">%02.1f%%</td>' % cumpct)
                    f.write (  '<td class="lpad">&lt;Unknown&gt;</td>')
                    f.write (  '<td class="lpad">%s</td>' % htquote(func))
                    f.write ('</tr>\n')

            if (cumpct > MAX_CUM_PCT): break
            if (stallpct < MIN_CONTRIB_PCT): break

    if (n_items > MAX_CONTRIB_ITEMS):
        f.write ('<tr>')
        f.write (  '<td colspan=6 class="lpad"><i>Note: Only top %d of %d %s are displayed</i></td>' % (MAX_CONTRIB_ITEMS, n_items, which))
        f.write ('</tr>')

    f.write ('            </table>\n')
    f.write ('          </div>\n')



def pc_to_pc(pc,pc_to_func): return pc



def pc_to_fname(pc,pc_to_func): return pc_to_func[pc]



def func_to_pc(func,pc_to_func):
    x = objdump.lookup(func)
    if x == None: x = "?"
    return x



def func_to_fname(func,pc_to_func): return func



def markstall(name, displayname, processor, packet_profile_help, bold=False):
    if stathist.data:
        description = stathist.data.get(packet_profile_help, {}).get(name, {}).get("Description", "no help available")
        hint = stathist.data.get(packet_profile_help, {}).get(name, {}).get("Hint_to_programmer", "none")
        titleOpt = description + "\nHint: " + hint
    else:
        titleOpt = "no help available"
    if name == "cycles":
        titleOpt = "Total count of stalls and commits"
    elif name == "stall_total":
        titleOpt = "Total stall count"
    elif name == "commits":
        titleOpt = "Total commit count"
    classes = "rightpane_select"
    if (bold):
        classes += " bold"
    fontColor = myColor(name)
    if (fontColor != ''):
        colorstr = ' style="color:' + fontColor + '"'
    else:
        colorstr = ''
    return '<span class="%s" data-target="%s_rightpane_%s" title="%s"%s>%s</span>' % (classes, processor, name, titleOpt, colorstr, displayname)



def gen_topstalls(f,pc_to_func,stalls_funcs,perstall_stats, processor, packet_profile_help):
    # Emit top stalls
    topstalls = {}

    for stall,pcvaldict in perstall_stats.items():
        topstalls[stall] = sum(pcvaldict.values())

    cycles = topstalls['cycles']

    stalltop = sorted (sorted (topstalls.items(), key=lambda x: str(x[0])), key=lambda x: x[1], reverse=True)

    f.write ('\n')
    f.write ('            <table id="%s_lefttable" class="lefttable">\n' % processor)
    f.write ('              <tr>\n')
    f.write ('                <td class="lg_hdr">Summary</td>\n')
    f.write ('                <td class="sm_hdr_lpad" title="Count of stalls and commits.\nThe sum of \'Stalls\' and \'Commits\' equals \'Total\'.">Count</td>\n')
    f.write ('                <td class="sm_hdr_lpad" title="Percentage of total counts attributable to commits and stalls.\nThe sum of \'Stalls\' and \'Commits\' equals \'Total\'.">Pct</td>\n')
    f.write ('              </tr>\n')

    for name,stalls in stalltop:
        if   name == "cycles"      : displayname = "Total"
        else: continue

        stallpct = 100.0*stalls/cycles

        f.write ('              <tr>')
        f.write (                '<td>%s</td>\n' % markstall(name, displayname, processor, packet_profile_help))
        f.write (                '<td class="a_lpad">%d</td>' % stalls)
        f.write (                '<td class="a_lpad" style="color:%s">%02.1f%%</td>' % (myColor(name), stallpct))
        f.write (              '</tr>\n')

    found_stall_total = False
    found_commits     = False

    for name,stalls in stalltop:
        if   name == "stall_total" : displayname = "Stalls";  found_stall_total = True
        elif name == "commits"     : displayname = "Commits"; found_commits     = True
        else: continue

        stallpct = 100.0*stalls/cycles

        f.write ('              <tr>')
        f.write (                '<td>%s</td>' % markstall(name, displayname, processor, packet_profile_help))
        f.write (                '<td class="a_lpad">%d</td>' % stalls)
        f.write (                '<td class="a_lpad" style="color:%s">%02.1f%%</td>' % (myColor(name), stallpct))
        f.write (              '</tr>\n')

        if found_stall_total == True:
            if found_commits == True:
                if stalls > 0:
                    f.write ('              <tr>\n')
                    f.write ('                <td class="lg_hdr">Stall Types</td>\n')
                    f.write ('                <td class="sm_hdr_lpad" title="Count of each stall type.\nThe sum of this column equals the \'Stalls\' count above.">Count</td>\n')
                    f.write ('                <td class="sm_hdr_lpad" title="Percentage of total counts attributable to each stall type.\nThe sum of this column equals the \'Stalls\' percentage above.">Pct</td>\n')
                    f.write ('              </tr>\n')
                else:
                    f.write ('              <tr>\n')
                    f.write ('                <td colspan=3 class="lg_hdr" style="color:%s">No stall information was found.<br>Ensure timing is enabled: \'hexagon-sim --timing\'</td>\n' % myColor('error'))
                    f.write ('              </tr>\n')
                break;

    for name,stalls in stalltop:
        stallpct = 100.0*stalls/cycles

        if   name == "cycles"      : continue
        elif name == "stall_total" : continue
        elif name == "commits"     : continue

        f.write ('              <tr>')
        f.write (                '<td>%s</td>' % markstall(name, name, processor, packet_profile_help))
        f.write (                '<td class="a_lpad">%d</td>' % stalls)
        f.write (                '<td class="a_lpad">%02.1f%%</td>' % stallpct)
        f.write (              '</tr>\n')

    f.write ('            </table>\n')

    f.write (boilerplate_topbetween.format(processor))

    f.write ('            <table style="vertical-align:top;">\n')
    f.write ('              <tr>\n')
    f.write ('                <td><input type="radio" name="%s_funcag" value="3" style="vertical-align:text-bottom;" checked><b>Top Packets</b></td>\n' % processor)
    f.write ('                <td><input type="radio" name="%s_funcag" value="2" style="vertical-align:text-bottom;"        ><b>Top Functions &nbsp;</b></td>\n' % processor)
    f.write ('              </tr>\n')
    f.write ('            </table>\n')

    for name,stalls in stalltop:
        gen_stallinfo(f,name,stalls,pc_to_func,perstall_stats[name],False,pc_to_pc,pc_to_fname, processor)
        gen_stallinfo(f,name,stalls,pc_to_func,stalls_funcs[name],True,func_to_pc,func_to_fname, processor)



def gen_topevents(f,pc_to_func,stalls_funcs,event_stats, processor, packet_profile_help):
    # Emit top events
    topevents = {}

    for event,pcvaldict in event_stats.items():
        topevents[event] = sum(pcvaldict.values())

    eventtop = sorted (sorted(topevents.items(), key=lambda x: x[0]), key=lambda x: x[1], reverse=True)

    f.write ('\n')
    f.write ('            <table id="events_lefttable" class="lefttable">\n')
    f.write ('              <tr>\n')
    f.write ('                <td class="lg_hdr" title="PMU events attributable to specific instructions">PMU Events</td>\n')
    f.write ('                <td class="sm_hdr_lpad" title="Count of events">Count</td>\n')
    f.write ('              </tr>\n')

    for name,events in eventtop:
        f.write ('              <tr>')
        f.write ('                <td>%s</td>' % markstall(name, name, processor, packet_profile_help))
        f.write ('                <td class="a_lpad">%d</td>' % events)
        f.write ('              </tr>\n')

    f.write ('            </table>\n')

    f.write (boilerplate_topbetween.format(processor))

    f.write ('            <table style="vertical-align:top;">\n')
    f.write ('              <tr>\n')
    f.write ('                <td><input type="radio" name="ev_funcag" value="3" style="vertical-align:text-bottom;" checked><b>Top Packets</b></td>\n')
    f.write ('                <td><input type="radio" name="ev_funcag" value="2" style="vertical-align:text-bottom;"        ><b>Top Functions &nbsp;</b></td>\n')
    f.write ('              </tr>\n')
    f.write ('            </table>\n')

    for name,events in eventtop:
        gen_stallinfo(f,name,events,pc_to_func,event_stats[name],False,pc_to_pc,pc_to_fname, processor)
        gen_stallinfo(f,name,events,pc_to_func,stalls_funcs[name],True,func_to_pc,func_to_fname, processor)



# Emit annotated disassembly

def print_stallinfo(f,pc,pc_to_func,pc_stats,pc_stats_events,processor,row_id,prev_filenum,prev_linenum):
    stall_info = pc_stats.get(pc,{})
    event_info = pc_stats_events.get(pc,{})

    cycles = stall_info.get('cycles',0)

    stalls = sorted (sorted (stall_info.items(), key=lambda x: str(x[0])), key=lambda x: x[1], reverse=True)
    events = sorted (sorted (event_info.items(), key=lambda x: str(x[0])), key=lambda x: x[1], reverse=True)

    func = pc_to_func.get(pc,"?")
    funcIndex = 0
    for i in range (0, len(objdump.funcs)):
        if (func == objdump.funcs[i][2]):
            if ((pc >= objdump.funcs[i][0]) and (pc < objdump.funcs[i][1])):
                funcIndex = i + 2
                break

    disasm = objdump.disdict.get(pc,'-').replace('\n',' ').replace('{','').replace('}','').strip()
    hll_info = objdump.hlldict.get(pc,'')

    if (hll_info != ''):
        parts = hll_info.split(':')
        if (len(parts) == 2):
            hll_filenum = int(parts[0],10)
            hll_linenum = int(parts[1],10)

            if ((hll_filenum != prev_filenum) or (hll_linenum != prev_linenum)):
                f.write ('{id:%d,l:1,p:%d,f:1,d:" ",c:%d},\n' % (row_id, hll_filenum, hll_linenum))
                row_id += 1
                prev_filenum = hll_filenum
                prev_linenum = hll_linenum

    # id, level:1, pc, funcIndex, disasm, cycles
    f.write ('{id:%d,l:1,p:0x%x,f:%d,d:"%s",c:%d},\n' % (row_id, pc, funcIndex, disasm, cycles))
    row_id += 1

    if cycles:
        # id, level:2, %bar, %age, "commits", num_commits
        # id, level:2, %bar, %age, "stalls",  num_stalls
        for thing in [ "commits", "stall_total" ]:
            num = stall_info.get(thing,0)
            frac = float(num)/cycles
            f.write ('{id:%d,l:2,p:%02.1f,d:"%s",c:%d},\n' % (row_id, 100*frac, thing, num))
            row_id += 1

        if stall_info.get('stall_total',0):
            # id, level:3, %bar, %age, stallname, num_stalls
            # ...
            for name,num in stalls:
                if name in [ "cycles", "commits", "stall_total" ]: continue
                frac = float(num)/cycles
                f.write ('{id:%d,l:3,p:%02.1f,d:"%s",c:%d,r:%s},\n' % (row_id,100*frac,name,num,stathist.data.get("{0}_packet_profile_help".format(processor.lower()), {}).get(name, {}).get("Stall_reduction_potential", 100) if stathist.data else 0))
                row_id += 1;

        # id, level:5, eventname, num_events
        # ...
        for name,num in events:
            f.write ('{id:%d,l:5,d:"%s",c:%d},\n' % (row_id,name,num))
            row_id += 1;
    return (row_id, prev_filenum, prev_linenum)


def gen_disasm(f,all_pcs,pc_to_func,pc_stats,pc_stats_events,processor):
    row_id = 0
    prev_filenum = 0
    prev_linenum = 0
    pc_array = []
    id_array = []

    f.write ('\n  data.%s = [\n' % processor)

    for pc in all_pcs:
        pc_array.append(pc)
        id_array.append(row_id)
        (row_id, prev_filenum, prev_linenum) = print_stallinfo(f,pc,pc_to_func,pc_stats,pc_stats_events,processor,row_id,prev_filenum,prev_linenum)

    f.write ('];\n\n')

    f.write ('  PC_to_IDX.%s = {\n' % processor)

    for i in range (0, len(pc_array)):
        f.write ('"%x":%d,' % (pc_array[i], id_array[i]))

    f.write ('};\n')


def die_usage():
    # only used when running as a standalone python script
    print ("Usage: python %s --packet_analyze <pkt_profile.json> <hexagon.elf> <pkt_profile.html>" % os.path.basename(sys.argv[0]))
    sys.exit(1)



def filter_idlefuncs(all_pcs,pc_to_func,pc_stats):
    non_idle_pcs = set([ pc for pc in all_pcs if pc in pc_stats ])
    non_idle_funcs = set([ pc_to_func.get(pc,'?') for pc in non_idle_pcs ])
    non_idle_funcs.discard("<unknown>")
    return [ pc for pc in all_pcs if (pc in non_idle_pcs) or (pc_to_func.get(pc,'?') in non_idle_funcs) ]



def function_aggregate(pc_stats,pc_to_func):
    funcstalls = {}
    stallfuncs = {}
    for pc,data in pc_stats.items():
        func = pc_to_func.get(pc,"unknown")
        for stat,count in data.items():
            fstallhist = funcstalls.setdefault(func,{})
            fstallhist[stat] = fstallhist.get(stat,0) + count
    for func,data in funcstalls.items():
        for stat,count in data.items():
            funchist = stallfuncs.setdefault(stat,{})
            funchist[func] = funchist.get(func,0) + count
    return (funcstalls,stallfuncs)



def print_ihist_table(f,name,total_count,insn_syntax,sorted_ihist):
    f.write     ('          <table><caption><b>Instructions Executed (by {0})</b><p></caption>\n'.format(name))
    f.write     ('            <tr>\n')
    f.write     ('              <th>#</th>\n')
    f.write     ('              <th class="rlpad">Tag</th>\n')
    if (insn_syntax):
        f.write ('              <th class="rlpad">Syntax</th>\n')
    f.write     ('              <th>Count</th>\n')
    f.write     ('              <th>Pct</th>\n')
    f.write     ('            </tr>\n')

    index = 0

    for item in sorted_ihist:
        tag    = item[0]
        if (insn_syntax):
            count  = item[1]['count']
            syntax = item[1]['syntax']
            syntax = syntax.replace('&', '&amp;')
            syntax = syntax.replace('<', '&lt;')
            syntax = syntax.replace('>', '&gt;')
        else:
            count = item[1]
        insn_pct = "%03.3f%%" % ((100.0*count)/total_count)
        f.write     ('            <tr>')
        f.write     (              '<th class="a_rlpad">%d</th>' % index)
        f.write     (              '<td class="rlpad">%s</td>'   % tag)
        if (insn_syntax):
            f.write (              '<td class="rlpad">%s</td>'   % syntax)
        f.write     (              '<td class="a_rlpad">%d</td>' % count)
        f.write     (              '<td class="a_rlpad">%s</td>' % insn_pct)
        f.write     (            '</tr>\n')
        index += 1

    f.write     ('            <tr>')
    f.write     (              '<th></th>')
    if (insn_syntax):
        f.write (              '<th colspan=2 class="rlpad" style="text-align:left">Total Count</th>')
    else:
        f.write (              '<th class="rlpad" style="text-align:left">Total Count</th>')
    f.write     (              '<th class="a_rlpad">{0}</th>'.format(total_count))
    f.write     (              '<th class="a_rlpad">100%</th>')
    f.write     (            '</tr>\n')
    f.write     ('          </table><br>\n')



def print_ihist(f,ihist):
    if (((stathist.majorVersion == 2) and (stathist.minorVersion >= 4)) or (stathist.majorVersion >= 3)):
        insn_syntax = True
    else:
        insn_syntax = False

    sorted_ihist_by_tag  = sorted (ihist.items(), key=lambda x: x[0])

    if (insn_syntax):
        sorted_ihist_by_syntax = sorted (sorted(ihist.items(), key=lambda x: x[0]), key=lambda x: x[1]['syntax'])
        sorted_ihist_by_count  = sorted (sorted(ihist.items(), key=lambda x: x[0]), key=lambda x: x[1]['count'], reverse=True)
    else:
        sorted_ihist_by_count  = sorted (sorted(ihist.items(), key=lambda x: x[0]), key=lambda x: x[1], reverse=True)

    total_count = 0

    if (insn_syntax):
        for (item) in sorted_ihist_by_count:
            total_count += item[1]['count']
    else:
        for (i, count) in sorted_ihist_by_count:
            total_count += count

    f.write ('\n')
    f.write ('    <div id="ihist" class="proctab" style="margin-top: 0.5em">\n')
    f.write ('      <div style="height:100%;overflow-y:auto">\n')
    f.write ('        <div style="display:inline-block; margin:0.5em;">\n')

    print_ihist_table (f, "Count",total_count,insn_syntax,sorted_ihist_by_count)

    if (insn_syntax):
        f.write ('        </div>\n')
        f.write ('        <div style="display:inline-block; margin:0.5em;">\n')

        print_ihist_table (f,"Syntax",total_count,insn_syntax,sorted_ihist_by_syntax)

    f.write ('        </div>\n')
    f.write ('        <div style="display:inline-block; margin:0.5em;">\n')

    print_ihist_table (f,"Tag",total_count,insn_syntax,sorted_ihist_by_tag)

    f.write ('        </div>\n')
    f.write ('      </div>\n')
    f.write ('    </div>\n')



def print_pmu_stats(f, pmu_stats):
    f.write ('\n')
    f.write ('    <div id="stats" class="proctab" style="padding: 1em 1.4em 0 1.4em">\n')
    f.write ('      <div style="height:100%;overflow-y:scroll">\n')
    f.write ('        <table>\n')
    f.write ('          <tr>\n')
    f.write ('            <th class="rlpad">Index</th>\n')
    f.write ('            <th>Name</th>\n')
    f.write ('            <th>Count</th>\n')
    f.write ('            <th>Count Per-packet</th>\n')
    f.write ('          </tr>\n')

    #sort the pmu stats first
    sorted_pmu_stats = sorted (pmu_stats.items(), key=lambda x: int(x[0], 0))

    for (event_idx, stat_detail) in sorted_pmu_stats:
        f.write ('          <tr title="{0}">'.format(stat_detail['Description']))
        f.write (            '<th class="a_rpad">{0}</th>'.format(event_idx))
        f.write (            '<td>{0}</td>'.format(stat_detail['Name']))
        f.write (            '<td class="a_nopad">{0}</td>'.format(stat_detail['Value']))
        f.write (            '<td class="a_nopad">{0}</td>'.format(stat_detail.get('Value Per Packet', '')))
        f.write (          '</tr>\n')

    f.write ('        </table>\n')
    f.write ('      </div>\n')
    f.write ('    </div>\n')



def print_derived_stats(f, derived_stats):
    f.write ('\n')
    f.write ('    <div id="derived_stats" class="proctab" style="padding: 1em 1.4em 0 1.4em">\n')
    f.write ('      <div style="height:100%;overflow-y:scroll">\n')
    f.write ('        <table>\n')
    f.write ('          <tr>\n')
    f.write ('            <th>Name</th>\n')
    f.write ('            <th>Count</th>\n')
    f.write ('            <th>Equation</th>\n')
    f.write ('          </tr>\n')

    #sort the derived stats first
    sorted_derived_stats = sorted (derived_stats.items(), key=lambda x: x[0])

    for (stat_name, stat_detail) in sorted_derived_stats:
        f.write ('          <tr title="{0}">'.format(stat_detail['Description']))
        f.write (            '<td>{0}</td>'.format(stat_name))
        f.write (            '<td class="a_nopad">{0}</td>'.format(stat_detail['Value']))
        f.write (            '<td>{0}</td>'.format(stat_detail['Equation']))
        f.write (          '</tr>\n')

    f.write ('        </table>\n')
    f.write ('      </div>\n')
    f.write ('    </div>\n')



def print_help(f):
    f.write ('\n')
    f.write ('    <div id="Help" class="proctab" style="padding: 1em 1.4em 0 1.4em">\n')
    f.write ('      <div style="height:100%;overflow-y:auto">\n')
    f.write ('        <p><b>NOTE:</b> On each clock tick, the activity on every thread is accumulated. Assume for example a 1GHz core with 4 threads that simulates for 1 second but only 1 thread is active and the others are in WAIT mode. Further assume the active thread spends half its time committing packets and half the time stalled. In this case the cycles*4 will be 4 billion, WAIT_CYCLES will be 3 billion, commits will be 500 million, and the total of all other stall types will be 500 million.</p>\n')
    f.write ('        <b>Simulation Settings:</b>\n')

    if stathist.data:
        f.write ('        <table>\n')
        f.write ('          <tr>\n')
        f.write ('            <td style="vertical-align:top;white-space:nowrap;padding-right:10px;">Profile data version:</td>\n')
        f.write ('            <td style="vertical-align:top;">%s</td>\n' % stathist.data['version'])
        f.write ('          </tr>\n')

        sorted_data = sorted (stathist.data["siminfo"].items(), key=lambda x: x[0])

        for setting,value in sorted_data:
            f.write ('          <tr>\n')
            f.write ('            <td style="vertical-align:top;white-space:nowrap;">%s:</td>\n' % setting)
            f.write ('            <td style="vertical-align:top;">%s</td>\n' % value)
            f.write ('          </tr>\n')
        f.write ('        </table>\n')
    else:
        f.write ('Simulation settings are unavailable\n')

    f.write ('      </div>\n')
    f.write ('    </div>\n')


def str2bool(str):
    return str.lower() in ('true', 't', 'yes', '1')

showLeadingZeros = False
show_0x          = False
s0x              = ''


def do_main(cmd,flag,fn_json,fn_elf,fn_html,tools_dir,vStr,slzStr,s0xStr,toolVersion,braceStr,jtfStr,fixChromeMagStr,dockStr,ulJumpsStr,hiJumpsStr,selStr):
    global showLeadingZeros
    global show_0x
    global s0x
    global fixChromeMagnify

    startpc = None
    stoppc = None
    pkt_profile = False

    verbose = str2bool (vStr)
    show_0x = str2bool (s0xStr)
    showLeadingZeros = str2bool (slzStr)
    showPacketBraces = str2bool (braceStr)
    showJumpToFuncs  = str2bool (jtfStr)
    fixChromeMagnify = str2bool (fixChromeMagStr)
    underlineJumps   = str2bool (ulJumpsStr)
    highlightJumps   = str2bool (hiJumpsStr)
    dockMenu         = str2bool (dockStr)
    showSelectedType = str2bool (selStr)

    sel = 'true' if showSelectedType else 'false'
    dkm = 'true' if dockMenu         else 'false'
    fcm = 'true' if fixChromeMagnify else 'false'
    hlj = 'true' if highlightJumps   else 'false'
    ulj = 'true' if underlineJumps   else 'false'
    jtf = 'true' if showJumpToFuncs  else 'false'
    spb = 'true' if showPacketBraces else 'false'
    slz = 'true' if showLeadingZeros else 'false'
    s0x = '0x'   if show_0x          else ''

    fcm_state = 'checked' if fixChromeMagnify else ''
    hlj_state = 'checked' if highlightJumps   else ''
    ulj_state = 'checked' if underlineJumps   else ''
    jtf_state = 'checked' if showJumpToFuncs  else ''
    spb_state = 'checked' if showPacketBraces else ''
    slz_state = 'checked' if showLeadingZeros else ''
    s0x_state = 'checked' if show_0x          else ''
    sel_state = 'checked' if showSelectedType else ''

    fcm_action = 'use'  if fixChromeMagnify else 'override'
    hlj_action = 'hide' if highlightJumps   else 'show'
    ulj_action = 'hide' if underlineJumps   else 'show'
    jtf_action = 'hide' if showJumpToFuncs  else 'show'
    spb_action = 'hide' if showPacketBraces else 'show'
    slz_action = 'hide' if showLeadingZeros else 'show'
    s0x_action = 'hide' if show_0x          else 'show'
    sel_action = 'hide' if showSelectedType else 'show'

    if (verbose):
        print ("tools_dir = %s"      % tools_dir)
        print ("verbose=%s"          % verbose)
        print ("showLeadingZeros=%s" % showLeadingZeros)
        print ("showPacketBraces=%s" % showPacketBraces)
        print ("showJumpToFuncs=%s"  % showJumpToFuncs)
        print ("show_0x=%s"          % show_0x)
        print ("fixChromeMagnify=%s" % fixChromeMagnify)
        print ("dockMenu=%s"         % dockMenu)
        print ("underlineJumps=%s"   % underlineJumps)
        print ("highlightJumps=%s"   % highlightJumps)
        print ("showSelectedType=%s" % showSelectedType)

    if   flag == "--packet_analyze": pkt_profile = True
    elif flag == "--stathist":       pkt_profile = True
    elif flag == "--uarchtrace":     sys.exit("The --uarchtrace option is not yet supported")
    elif flag == "--help":           die_usage()
    else:
        print ("proftool.py: invalid option '%s'" % flag)
        return

    if os.path.isfile(fn_json) == False:
        print ("proftool.py: %s: File not found" % fn_json)
        print ("To create file '%s', run 'hexagon-sim --timing --packet_analyze %s <test.elf>'" % (fn_json, fn_json))
        return

    hexobjs = fn_elf.split(',')

    # FIXME: why do we iterate here only to discard the results?
    for arg in hexobjs:
        fn, obj = objdump.get_elf_args(arg)
        if os.path.isfile(fn) == False:
            print ("proftool.py: %s: File not found" % fn)
            return

    try:
        f_in = open(fn_json,"r")
    except:
        print ("proftool.py: %s: Cannot open file for reading" % fn_json)
        return

    try:
        if (sys.version_info[0] < 3):
            f = open(fn_html,"wb")
        else:
            f = open(fn_html,"w",newline='\n')
    except:
        print ("proftool.py: %s: Cannot open file for writing" % fn_html)
        return

    objdump.get_info(fn_elf, tools_dir, verbose)
    f.write (boilerplate_start.format(fn_elf))
    f.write (boilerplate_start2)

    stathist.read_pa(f_in)
    include_derived = 'derived_stats' in stathist.data and stathist.data['derived_stats']
    processors = [stathist.data['packet_profiles'][p] for p in stathist.data['packet_profiles']]
    processors = sorted (processors, key=lambda x: x[0])

    f.write ('    <ul>\n')
    f.write ('      <li>\n')
    f.write ('        <div class="hamburger" id="hamburger1" title="Menu" onclick="openNav()" onmouseover="hbFocus()" onmouseout="hbFade()">\n')
    f.write ('          <div id="hbBar1" class="hbBar"></div>\n')
    f.write ('          <div id="hbBar2" class="hbBar"></div>\n')
    f.write ('          <div id="hbBar3" class="hbBar"></div>\n')
    f.write ('        </div>\n')
    f.write ('      </li>\n')
    for p in processors:
        f.write ('      <li><a href="#{0}">{0} Stalls</a></li>\n'.format(p))
    f.write ('      <li><a href="#events">Events</a></li>\n')
    f.write ('      <li><a href="#stats">PMU Events</a></li>\n')
    if include_derived:
        f.write ('      <li><a href="#derived_stats">Derived Stats</a></li>\n')
    f.write ('      <li><a href="#ihist">Instructions</a></li>\n')
    f.write ('      <li><a href="#Help">Help</a></li>\n')
    f.write ('      <li><div id="searchContainer"><input type="search" name="mysearch" id="search1" placeholder="Search grid ..."><span class="clearSearch" id="clearSearch1">&times;</span></div>')
    f.write ('    </ul>\n')

    data_perproc = {}
    for (packet_profile, processor) in stathist.data['packet_profiles'].items():
        if pkt_profile == True: (pc_stats, pc_stats_events) = stathist.parse(packet_profile)
        else:
            die_usage()
            return

        # create reverse dictionaries
        perstall_stats = {}
        for pc,stalldict in pc_stats.items():
            for stall,val in stalldict.items():
                stalldict = perstall_stats.setdefault(stall,{})
                stalldict[pc] = val + stalldict.get(pc,0)
        event_stats = {}
        for pc,eventdict in  pc_stats_events.items():
            for event,val in eventdict.items():
                if val != 0:
                    edict = event_stats.setdefault(event, {})
                    edict[pc] = val + edict.get(pc, 0)
        # find all program counters: either things we executed or things we got from the ELF
        disdict_pcs = set(objdump.disdict.keys())
        sim_pcs = set(pc_stats.keys())
        all_pcs = sorted(list(disdict_pcs | sim_pcs))
        pc_to_func = objdump.pc_to_funcs(all_pcs)
        # Filter out never-executed functions / address ranges
        all_pcs = filter_idlefuncs(all_pcs,pc_to_func,pc_stats)
        (funcs_stalls,stalls_funcs) = function_aggregate(pc_stats,pc_to_func)

        f.write (boilerplate_topleft.format(processor))

        gen_topstalls(f,pc_to_func,stalls_funcs,perstall_stats, processor, packet_profile + "_help")

        f.write (boilerplate_topright.format(processor))

        if processor == "CORE":
            (funcs_events, stalls_events) = function_aggregate(pc_stats_events, pc_to_func)
            f.write (boilerplate_topleft_events.format("events"))
            gen_topevents(f, pc_to_func, stalls_events, event_stats, "events", "core_packet_profile_help");
            f.write (boilerplate_topright.format("events"))

        data_perproc[processor] = {
            "all_pcs": all_pcs,
            "pc_to_func": pc_to_func,
            "pc_stats": pc_stats,
            "pc_stats_events": pc_stats_events
        }

    print_pmu_stats(f, stathist.data['stats'])
    if include_derived:
        print_derived_stats(f, stathist.data['derived_stats'])
    if 'ihist' in stathist.data:
        print_ihist(f, stathist.data['ihist'])
    print_help(f)

    f.write (boilerplate_mid.format(js_blob,slz,s0x,spb,jtf,fcm,dkm,ulj,hlj,sel))
    f.write (boilerplate_mid2)
    for p in processors:
        d = data_perproc[p]
        gen_disasm(f,
                   d["all_pcs"],
                   d["pc_to_func"],
                   d["pc_stats"],
                   d["pc_stats_events"],
                   p)

    f.write ('\n  const FileNames = [""')
    for i in range (1, len(objdump.filenames)):
        f.write (',\n"%s"' % objdump.filenames[i])
    f.write ('];\n')

    f.write ('\n  const FuncNames = [["",0,0],["HLL",0,0]')
    for i in range (0, len(objdump.funcs)):
        f.write (',\n["%s",0x%x,0x%x]' % (objdump.funcs[i][2], objdump.funcs[i][0], objdump.funcs[i][1]))
    f.write ('];\n')

    f.write (boilerplate_mid3)
    f.write (boilerplate_end.format(toolVersion,jtf_state,jtf_action,spb_state,spb_action,slz_state,slz_action,s0x_state,s0x_action,ulj_state,ulj_action,hlj_state,hlj_action,sel_state,sel_action))












# Just a javascript blob and calling do_main below here...


# Seriously, you should probably stop scrolling



























































js_blob = r"""
(function(e){e.fn.drag=function(t,n,r){var i=typeof t=="string"?t:"",s=e.isFunction(t)?t:e.isFunction(n)?n:null;if(i.indexOf("drag")!==0)i="drag"+i;r=(t==s?n:r)||{};return s?this.bind(i,r,s):this.trigger(i)};var t=e.event,n=t.special,r=n.drag={defaults:{which:1,distance:0,not:":input",handle:null,relative:false,drop:true,click:false},datakey:"dragdata",noBubble:true,add:function(t){var n=e.data(this,r.datakey),i=t.data||{};n.related+=1;e.each(r.defaults,function(e,t){if(i[e]!==undefined)n[e]=i[e]})},remove:function(){e.data(this,r.datakey).related-=1},setup:function(){if(e.data(this,r.datakey))return;var n=e.extend({related:0},r.defaults);e.data(this,r.datakey,n);t.add(this,"touchstart mousedown",r.init,n);if(this.attachEvent)this.attachEvent("ondragstart",r.dontstart)},teardown:function(){var n=e.data(this,r.datakey)||{};if(n.related)return;e.removeData(this,r.datakey);t.remove(this,"touchstart mousedown",r.init);r.textselect(true);if(this.detachEvent)this.detachEvent("ondragstart",r.dontstart)},init:function(i){if(r.touched)return;var s=i.data,o;if(i.which!=0&&s.which>0&&i.which!=s.which)return;if(e(i.target).is(s.not))return;if(s.handle&&!e(i.target).closest(s.handle,i.currentTarget).length)return;r.touched=i.type=="touchstart"?this:null;s.propagates=1;s.mousedown=this;s.interactions=[r.interaction(this,s)];s.target=i.target;s.pageX=i.pageX;s.pageY=i.pageY;s.dragging=null;o=r.hijack(i,"draginit",s);if(!s.propagates)return;o=r.flatten(o);if(o&&o.length){s.interactions=[];e.each(o,function(){s.interactions.push(r.interaction(this,s))})}s.propagates=s.interactions.length;if(s.drop!==false&&n.drop)n.drop.handler(i,s);r.textselect(false);if(r.touched)t.add(r.touched,"touchmove touchend",r.handler,s);else t.add(document,"mousemove mouseup",r.handler,s);if(!r.touched||s.live)return false},interaction:function(t,n){var i=e(t)[n.relative?"position":"offset"]()||{top:0,left:0};return{drag:t,callback:new r.callback,droppable:[],offset:i}},handler:function(i){var s=i.data;switch(i.type){case!s.dragging&&"touchmove":i.preventDefault();case!s.dragging&&"mousemove":if(Math.pow(i.pageX-s.pageX,2)+Math.pow(i.pageY-s.pageY,2)<Math.pow(s.distance,2))break;i.target=s.target;r.hijack(i,"dragstart",s);if(s.propagates)s.dragging=true;case"touchmove":i.preventDefault();case"mousemove":if(s.dragging){r.hijack(i,"drag",s);if(s.propagates){if(s.drop!==false&&n.drop)n.drop.handler(i,s);break}i.type="mouseup"};case"touchend":case"mouseup":default:if(r.touched)t.remove(r.touched,"touchmove touchend",r.handler);else t.remove(document,"mousemove mouseup",r.handler);if(s.dragging){if(s.drop!==false&&n.drop)n.drop.handler(i,s);r.hijack(i,"dragend",s)}r.textselect(true);if(s.click===false&&s.dragging)e.data(s.mousedown,"suppress.click",(new Date).getTime()+5);s.dragging=r.touched=false;break}},hijack:function(n,i,s,o,u){if(!s)return;var a={event:n.originalEvent,type:n.type},f=i.indexOf("drop")?"drag":"drop",l,c=o||0,h,p,d,v=!isNaN(o)?o:s.interactions.length;n.type=i;n.originalEvent=null;s.results=[];do if(h=s.interactions[c]){if(i!=="dragend"&&h.cancelled)continue;d=r.properties(n,s,h);h.results=[];e(u||h[f]||s.droppable).each(function(o,u){d.target=u;n.isPropagationStopped=function(){return false};l=u?t.dispatch.call(u,n,d):null;if(l===false){if(f=="drag"){h.cancelled=true;s.propagates-=1}if(i=="drop"){h[f][o]=null}}else if(i=="dropinit")h.droppable.push(r.element(l)||u);if(i=="dragstart")h.proxy=e(r.element(l)||h.drag)[0];h.results.push(l);delete n.result;if(i!=="dropinit")return l});s.results[c]=r.flatten(h.results);if(i=="dropinit")h.droppable=r.flatten(h.droppable);if(i=="dragstart"&&!h.cancelled)d.update()}while(++c<v);n.type=a.type;n.originalEvent=a.event;return r.flatten(s.results)},properties:function(e,t,n){var i=n.callback;i.drag=n.drag;i.proxy=n.proxy||n.drag;i.startX=t.pageX;i.startY=t.pageY;i.deltaX=e.pageX-t.pageX;i.deltaY=e.pageY-t.pageY;i.originalX=n.offset.left;i.originalY=n.offset.top;i.offsetX=i.originalX+i.deltaX;i.offsetY=i.originalY+i.deltaY;i.drop=r.flatten((n.drop||[]).slice());i.available=r.flatten((n.droppable||[]).slice());return i},element:function(e){if(e&&(e.jquery||e.nodeType==1))return e},flatten:function(t){return e.map(t,function(t){return t&&t.jquery?e.makeArray(t):t&&t.length?r.flatten(t):t})},textselect:function(t){e(document)[t?"unbind":"bind"]("selectstart",r.dontstart).css("MozUserSelect",t?"":"none");document.unselectable=t?"off":"on"},dontstart:function(){return false},callback:function(){}};r.callback.prototype={update:function(){if(n.drop&&this.available.length)e.each(this.available,function(e){n.drop.locate(this,e)})}};var i=t.dispatch;t.dispatch=function(t){if(e.data(this,"suppress."+t.type)-(new Date).getTime()>0){e.removeData(this,"suppress."+t.type);return}return i.apply(this,arguments)};var s=t.fixHooks.touchstart=t.fixHooks.touchmove=t.fixHooks.touchend=t.fixHooks.touchcancel={props:"clientX clientY pageX pageY screenX screenY".split(" "),filter:function(t,n){if(n){var r=n.touches&&n.touches[0]||n.changedTouches&&n.changedTouches[0]||null;if(r)e.each(s.props,function(e,n){t[n]=r[n]})}return t}};n.draginit=n.dragstart=n.dragend=r})(jQuery);(function(e){function t(){var e=false;var t=false;this.stopPropagation=function(){e=true};this.isPropagationStopped=function(){return e};this.stopImmediatePropagation=function(){t=true};this.isImmediatePropagationStopped=function(){return t}}function n(){var e=[];this.subscribe=function(t){e.push(t)};this.unsubscribe=function(t){for(var n=e.length-1;n>=0;n--){if(e[n]===t){e.splice(n,1)}}};this.notify=function(n,r,i){r=r||new t;i=i||this;var s;for(var o=0;o<e.length&&!(r.isPropagationStopped()||r.isImmediatePropagationStopped());o++){s=e[o].call(i,r,n)}return s}}function r(){var e=[];this.subscribe=function(t,n){e.push({event:t,handler:n});t.subscribe(n);return this};this.unsubscribe=function(t,n){var r=e.length;while(r--){if(e[r].event===t&&e[r].handler===n){e.splice(r,1);t.unsubscribe(n);return}}return this};this.unsubscribeAll=function(){var t=e.length;while(t--){e[t].event.unsubscribe(e[t].handler)}e=[];return this}}function i(e,t,n,r){if(n===undefined&&r===undefined){n=e;r=t}this.fromRow=Math.min(e,n);this.fromCell=Math.min(t,r);this.toRow=Math.max(e,n);this.toCell=Math.max(t,r);this.isSingleRow=function(){return this.fromRow==this.toRow};this.isSingleCell=function(){return this.fromRow==this.toRow&&this.fromCell==this.toCell};this.contains=function(e,t){return e>=this.fromRow&&e<=this.toRow&&t>=this.fromCell&&t<=this.toCell};this.toString=function(){if(this.isSingleCell()){return"("+this.fromRow+":"+this.fromCell+")"}else{return"("+this.fromRow+":"+this.fromCell+" - "+this.toRow+":"+this.toCell+")"}}}function s(){this.__nonDataRow=true}function o(){this.__group=true;this.level=0;this.count=0;this.value=null;this.title=null;this.collapsed=false;this.totals=null;this.rows=[];this.groups=null;this.groupingKey=null}function u(){this.__groupTotals=true;this.group=null;this.initialized=false}function a(){var e=null;this.isActive=function(t){return t?e===t:e!==null};this.activate=function(t){if(t===e){return}if(e!==null){throw"SlickGrid.EditorLock.activate: an editController is still active, can't activate another editController"}if(!t.commitCurrentEdit){throw"SlickGrid.EditorLock.activate: editController must implement .commitCurrentEdit()"}if(!t.cancelCurrentEdit){throw"SlickGrid.EditorLock.activate: editController must implement .cancelCurrentEdit()"}e=t};this.deactivate=function(t){if(e!==t){throw"SlickGrid.EditorLock.deactivate: specified editController is not the currently active one"}e=null};this.commitCurrentEdit=function(){return e?e.commitCurrentEdit():true};this.cancelCurrentEdit=function(){return e?e.cancelCurrentEdit():true}}e.extend(true,window,{Slick:{Event:n,EventData:t,EventHandler:r,Range:i,NonDataRow:s,Group:o,GroupTotals:u,EditorLock:a,GlobalEditorLock:new a}});o.prototype=new s;o.prototype.equals=function(e){return this.value===e.value&&this.count===e.count&&this.collapsed===e.collapsed&&this.title===e.title};u.prototype=new s})(jQuery);if(typeof jQuery==="undefined"){throw"SlickGrid requires jquery module to be loaded"}if(!jQuery.fn.drag){throw"SlickGrid requires jquery.event.drag module to be loaded"}if(typeof Slick==="undefined"){throw"slick.core.js not loaded"}(function($){function SlickGrid(container,data,columns,options){function init(){$container=$(container);if($container.length<1){throw new Error("SlickGrid requires a valid container, "+container+" does not exist in the DOM.")}maxSupportedCssHeight=maxSupportedCssHeight||getMaxSupportedCssHeight();scrollbarDimensions=scrollbarDimensions||measureScrollbar();options=$.extend({},defaults,options);validateAndEnforceOptions();columnDefaults.width=options.defaultColumnWidth;columnsById={};for(var e=0;e<columns.length;e++){var t=columns[e]=$.extend({},columnDefaults,columns[e]);columnsById[t.id]=e;if(t.minWidth&&t.width<t.minWidth){t.width=t.minWidth}if(t.maxWidth&&t.width>t.maxWidth){t.width=t.maxWidth}}if(options.enableColumnReorder&&!$.fn.sortable){throw new Error("SlickGrid's 'enableColumnReorder = true' option requires jquery-ui.sortable module to be loaded")}editController={commitCurrentEdit:commitCurrentEdit,cancelCurrentEdit:cancelCurrentEdit};$container.empty().css("overflow","hidden").css("outline",0).addClass(uid).addClass("ui-widget");if(!/relative|absolute|fixed/.test($container.css("position"))){$container.css("position","relative")}$focusSink=$("<div tabIndex='0' hideFocus style='position:fixed;width:0;height:0;top:0;left:0;outline:0;'></div>").appendTo($container);$headerScroller=$("<div class='slick-header ui-state-default' style='overflow:hidden;position:relative;' />").appendTo($container);$headers=$("<div class='slick-header-columns' style='left:-1000px' />").appendTo($headerScroller);$headers.width(getHeadersWidth());$headerRowScroller=$("<div class='slick-headerrow ui-state-default' style='overflow:hidden;position:relative;' />").appendTo($container);$headerRow=$("<div class='slick-headerrow-columns' />").appendTo($headerRowScroller);$headerRowSpacer=$("<div style='display:block;height:1px;position:absolute;top:0;left:0;'></div>").css("width",getCanvasWidth()+scrollbarDimensions.width+"px").appendTo($headerRowScroller);$topPanelScroller=$("<div class='slick-top-panel-scroller ui-state-default' style='overflow:hidden;position:relative;' />").appendTo($container);$topPanel=$("<div class='slick-top-panel' style='width:10000px' />").appendTo($topPanelScroller);if(!options.showTopPanel){$topPanelScroller.hide()}if(!options.showHeaderRow){$headerRowScroller.hide()}$viewport=$("<div class='slick-viewport' style='width:100%;overflow:auto;outline:0;position:relative;;'>").appendTo($container);$viewport.css("overflow-y",options.autoHeight?"hidden":"auto");$canvas=$("<div class='grid-canvas' />").appendTo($viewport);$focusSink2=$focusSink.clone().appendTo($container);if(!options.explicitInitialization){finishInitialization()}}function finishInitialization(){if(!initialized){initialized=true;viewportW=parseFloat($.css($container[0],"width",true));measureCellPaddingAndBorder();disableSelection($headers);if(!options.enableTextSelectionOnCells){$viewport.bind("selectstart.ui",function(e){return $(e.target).is("input,textarea")})}updateColumnCaches();createColumnHeaders();setupColumnSort();createCssRules();resizeCanvas();bindAncestorScrollEvents();$container.bind("resize.slickgrid",resizeCanvas);$viewport.bind("scroll",handleScroll);$headerScroller.bind("contextmenu",handleHeaderContextMenu).bind("click",handleHeaderClick).delegate(".slick-header-column","mouseenter",handleHeaderMouseEnter).delegate(".slick-header-column","mouseleave",handleHeaderMouseLeave);$headerRowScroller.bind("scroll",handleHeaderRowScroll);$focusSink.add($focusSink2).bind("keydown",handleKeyDown);$canvas.bind("keydown",handleKeyDown).bind("click",handleClick).bind("dblclick",handleDblClick).bind("contextmenu",handleContextMenu).bind("draginit",handleDragInit).bind("dragstart",{distance:3},handleDragStart).bind("drag",handleDrag).bind("dragend",handleDragEnd).delegate(".slick-cell","mouseenter",handleMouseEnter).delegate(".slick-cell","mouseleave",handleMouseLeave);if(navigator.userAgent.toLowerCase().match(/webkit/)&&navigator.userAgent.toLowerCase().match(/macintosh/)){$canvas.bind("mousewheel",handleMouseWheel)}}}function registerPlugin(e){plugins.unshift(e);e.init(self)}function unregisterPlugin(e){for(var t=plugins.length;t>=0;t--){if(plugins[t]===e){if(plugins[t].destroy){plugins[t].destroy()}plugins.splice(t,1);break}}}function setSelectionModel(e){if(selectionModel){selectionModel.onSelectedRangesChanged.unsubscribe(handleSelectedRangesChanged);if(selectionModel.destroy){selectionModel.destroy()}}selectionModel=e;if(selectionModel){selectionModel.init(self);selectionModel.onSelectedRangesChanged.subscribe(handleSelectedRangesChanged)}}function getSelectionModel(){return selectionModel}function getCanvasNode(){return $canvas[0]}function measureScrollbar(){var e=$("<div style='position:absolute; top:-10000px; left:-10000px; width:100px; height:100px; overflow:scroll;'></div>").appendTo("body");var t={width:e.width()-e[0].clientWidth,height:e.height()-e[0].clientHeight};e.remove();return t}function getHeadersWidth(){var e=0;for(var t=0,n=columns.length;t<n;t++){var r=columns[t].width;e+=r}e+=scrollbarDimensions.width;return Math.max(e,viewportW)+1e3}function getCanvasWidth(){var e=viewportHasVScroll?viewportW-scrollbarDimensions.width:viewportW;var t=0;var n=columns.length;while(n--){t+=columns[n].width}return options.fullWidthRows?Math.max(t,e):t}function updateCanvasWidth(e){var t=canvasWidth;canvasWidth=getCanvasWidth();if(canvasWidth!=t){$canvas.width(canvasWidth);$headerRow.width(canvasWidth);$headers.width(getHeadersWidth());viewportHasHScroll=canvasWidth>viewportW-scrollbarDimensions.width}$headerRowSpacer.width(canvasWidth+(viewportHasVScroll?scrollbarDimensions.width:0));if(canvasWidth!=t||e){applyColumnWidths()}}function disableSelection(e){if(e&&e.jquery){e.attr("unselectable","on").css("MozUserSelect","none").bind("selectstart.ui",function(){return false})}}function getMaxSupportedCssHeight(){var e=1e6;var t=navigator.userAgent.toLowerCase().match(/firefox/)?6e6:1e9;var n=$("<div style='display:none' />").appendTo(document.body);while(true){var r=e*2;n.css("height",r);if(r>t||n.height()!==r){break}else{e=r}}n.remove();return e}function bindAncestorScrollEvents(){var e=$canvas[0];while((e=e.parentNode)!=document.body&&e!=null){if(e==$viewport[0]||e.scrollWidth!=e.clientWidth||e.scrollHeight!=e.clientHeight){var t=$(e);if(!$boundAncestors){$boundAncestors=t}else{$boundAncestors=$boundAncestors.add(t)}t.bind("scroll."+uid,handleActiveCellPositionChange)}}}function unbindAncestorScrollEvents(){if(!$boundAncestors){return}$boundAncestors.unbind("scroll."+uid);$boundAncestors=null}function updateColumnHeader(e,t,n){if(!initialized){return}var r=getColumnIndex(e);if(r==null){return}var i=columns[r];var s=$headers.children().eq(r);if(s){if(t!==undefined){columns[r].name=t}if(n!==undefined){columns[r].toolTip=n}trigger(self.onBeforeHeaderCellDestroy,{node:s[0],column:i});s.attr("title",n||"").children().eq(0).html(t);trigger(self.onHeaderCellRendered,{node:s[0],column:i})}}function getHeaderRow(){return $headerRow[0]}function getHeaderRowColumn(e){var t=getColumnIndex(e);var n=$headerRow.children().eq(t);return n&&n[0]}function createColumnHeaders(){function e(){$(this).addClass("ui-state-hover")}function t(){$(this).removeClass("ui-state-hover")}$headers.find(".slick-header-column").each(function(){var e=$(this).data("column");if(e){trigger(self.onBeforeHeaderCellDestroy,{node:this,column:e})}});$headers.empty();$headers.width(getHeadersWidth());$headerRow.find(".slick-headerrow-column").each(function(){var e=$(this).data("column");if(e){trigger(self.onBeforeHeaderRowCellDestroy,{node:this,column:e})}});$headerRow.empty();for(var n=0;n<columns.length;n++){var r=columns[n];var i=$("<div class='ui-state-default slick-header-column' />").html("<span class='slick-column-name'>"+r.name+"</span>").width(r.width-headerColumnWidthDiff).attr("id",""+uid+r.id).attr("title",r.toolTip||"").data("column",r).addClass(r.headerCssClass||"").appendTo($headers);if(options.enableColumnReorder||r.sortable){i.on("mouseenter",e).on("mouseleave",t)}if(r.sortable){i.addClass("slick-header-sortable");i.append("<span class='slick-sort-indicator' />")}trigger(self.onHeaderCellRendered,{node:i[0],column:r});if(options.showHeaderRow){var s=$("<div class='ui-state-default slick-headerrow-column l"+n+" r"+n+"'></div>").data("column",r).appendTo($headerRow);trigger(self.onHeaderRowCellRendered,{node:s[0],column:r})}}setSortColumns(sortColumns);setupColumnResize();if(options.enableColumnReorder){setupColumnReorder()}}function setupColumnSort(){$headers.click(function(e){e.metaKey=e.metaKey||e.ctrlKey;if($(e.target).hasClass("slick-resizable-handle")){return}var t=$(e.target).closest(".slick-header-column");if(!t.length){return}var n=t.data("column");if(n.sortable){if(!getEditorLock().commitCurrentEdit()){return}var r=null;var i=0;for(;i<sortColumns.length;i++){if(sortColumns[i].columnId==n.id){r=sortColumns[i];r.sortAsc=!r.sortAsc;break}}if(e.metaKey&&options.multiColumnSort){if(r){sortColumns.splice(i,1)}}else{if(!e.shiftKey&&!e.metaKey||!options.multiColumnSort){sortColumns=[]}if(!r){r={columnId:n.id,sortAsc:n.defaultSortAsc};sortColumns.push(r)}else if(sortColumns.length==0){sortColumns.push(r)}}setSortColumns(sortColumns);if(!options.multiColumnSort){trigger(self.onSort,{multiColumnSort:false,sortCol:n,sortAsc:r.sortAsc},e)}else{trigger(self.onSort,{multiColumnSort:true,sortCols:$.map(sortColumns,function(e){return{sortCol:columns[getColumnIndex(e.columnId)],sortAsc:e.sortAsc}})},e)}}})}function setupColumnReorder(){$headers.filter(":ui-sortable").sortable("destroy");$headers.sortable({containment:"parent",distance:3,axis:"x",cursor:"default",tolerance:"intersection",helper:"clone",placeholder:"slick-sortable-placeholder ui-state-default slick-header-column",start:function(e,t){t.placeholder.width(t.helper.outerWidth()-headerColumnWidthDiff);$(t.helper).addClass("slick-header-column-active")},beforeStop:function(e,t){$(t.helper).removeClass("slick-header-column-active")},stop:function(e){if(!getEditorLock().commitCurrentEdit()){$(this).sortable("cancel");return}var t=$headers.sortable("toArray");var n=[];for(var r=0;r<t.length;r++){n.push(columns[getColumnIndex(t[r].replace(uid,""))])}setColumns(n);trigger(self.onColumnsReordered,{});e.stopPropagation();setupColumnResize()}})}function setupColumnResize(){var e,t,n,r,i,s,o,u,a;i=$headers.children();i.find(".slick-resizable-handle").remove();i.each(function(e,t){if(columns[e].resizable){if(u===undefined){u=e}a=e}});if(u===undefined){return}i.each(function(f,l){if(f<u||options.forceFitColumns&&f>=a){return}e=$(l);$("<div class='slick-resizable-handle' />").appendTo(l).bind("dragstart",function(e,u){if(!getEditorLock().commitCurrentEdit()){return false}r=e.pageX;$(this).parent().addClass("slick-header-column-active");var a=null,l=null;i.each(function(e,t){columns[e].previousWidth=$(t).outerWidth()});if(options.forceFitColumns){a=0;l=0;for(t=f+1;t<i.length;t++){n=columns[t];if(n.resizable){if(l!==null){if(n.maxWidth){l+=n.maxWidth-n.previousWidth}else{l=null}}a+=n.previousWidth-Math.max(n.minWidth||0,absoluteColumnMinWidth)}}}var h=0,p=0;for(t=0;t<=f;t++){n=columns[t];if(n.resizable){if(p!==null){if(n.maxWidth){p+=n.maxWidth-n.previousWidth}else{p=null}}h+=n.previousWidth-Math.max(n.minWidth||0,absoluteColumnMinWidth)}}if(a===null){a=1e5}if(h===null){h=1e5}if(l===null){l=1e5}if(p===null){p=1e5}o=r+Math.min(a,p);s=r-Math.min(h,l)}).bind("drag",function(e,u){var a,l=Math.min(o,Math.max(s,e.pageX))-r,h;if(l<0){h=l;for(t=f;t>=0;t--){n=columns[t];if(n.resizable){a=Math.max(n.minWidth||0,absoluteColumnMinWidth);if(h&&n.previousWidth+h<a){h+=n.previousWidth-a;n.width=a}else{n.width=n.previousWidth+h;h=0}}}if(options.forceFitColumns){h=-l;for(t=f+1;t<i.length;t++){n=columns[t];if(n.resizable){if(h&&n.maxWidth&&n.maxWidth-n.previousWidth<h){h-=n.maxWidth-n.previousWidth;n.width=n.maxWidth}else{n.width=n.previousWidth+h;h=0}}}}}else{h=l;for(t=f;t>=0;t--){n=columns[t];if(n.resizable){if(h&&n.maxWidth&&n.maxWidth-n.previousWidth<h){h-=n.maxWidth-n.previousWidth;n.width=n.maxWidth}else{n.width=n.previousWidth+h;h=0}}}if(options.forceFitColumns){h=-l;for(t=f+1;t<i.length;t++){n=columns[t];if(n.resizable){a=Math.max(n.minWidth||0,absoluteColumnMinWidth);if(h&&n.previousWidth+h<a){h+=n.previousWidth-a;n.width=a}else{n.width=n.previousWidth+h;h=0}}}}}applyColumnHeaderWidths();if(options.syncColumnCellResize){applyColumnWidths()}}).bind("dragend",function(e,r){var s;$(this).parent().removeClass("slick-header-column-active");for(t=0;t<i.length;t++){n=columns[t];s=$(i[t]).outerWidth();if(n.previousWidth!==s&&n.rerenderOnResize){invalidateAllRows()}}updateCanvasWidth(true);render();trigger(self.onColumnsResized,{})})})}function getVBoxDelta(e){var t=["borderTopWidth","borderBottomWidth","paddingTop","paddingBottom"];var n=0;$.each(t,function(t,r){n+=parseFloat(e.css(r))||0});return n}function measureCellPaddingAndBorder(){var e;var t=["borderLeftWidth","borderRightWidth","paddingLeft","paddingRight"];var n=["borderTopWidth","borderBottomWidth","paddingTop","paddingBottom"];e=$("<div class='ui-state-default slick-header-column' style='visibility:hidden'>-</div>").appendTo($headers);headerColumnWidthDiff=headerColumnHeightDiff=0;if(e.css("box-sizing")!="border-box"&&e.css("-moz-box-sizing")!="border-box"&&e.css("-webkit-box-sizing")!="border-box"){$.each(t,function(t,n){headerColumnWidthDiff+=parseFloat(e.css(n))||0});$.each(n,function(t,n){headerColumnHeightDiff+=parseFloat(e.css(n))||0})}e.remove();var r=$("<div class='slick-row' />").appendTo($canvas);e=$("<div class='slick-cell' id='' style='visibility:hidden'>-</div>").appendTo(r);cellWidthDiff=cellHeightDiff=0;if(e.css("box-sizing")!="border-box"&&e.css("-moz-box-sizing")!="border-box"&&e.css("-webkit-box-sizing")!="border-box"){$.each(t,function(t,n){cellWidthDiff+=parseFloat(e.css(n))||0});$.each(n,function(t,n){cellHeightDiff+=parseFloat(e.css(n))||0})}r.remove();absoluteColumnMinWidth=Math.max(headerColumnWidthDiff,cellWidthDiff)}function createCssRules(){$style=$("<style type='text/css' rel='stylesheet' />").appendTo($("head"));var e=options.rowHeight-cellHeightDiff;var t=["."+uid+" .slick-header-column { left: 1000px; }","."+uid+" .slick-top-panel { height:"+options.topPanelHeight+"px; }","."+uid+" .slick-headerrow-columns { height:"+options.headerRowHeight+"px; }","."+uid+" .slick-cell { height:"+e+"px; }","."+uid+" .slick-row { height:"+options.rowHeight+"px; }"];for(var n=0;n<columns.length;n++){t.push("."+uid+" .l"+n+" { }");t.push("."+uid+" .r"+n+" { }")}if($style[0].styleSheet){$style[0].styleSheet.cssText=t.join(" ")}else{$style[0].appendChild(document.createTextNode(t.join(" ")))}}function getColumnCssRules(e){if(!stylesheet){var t=document.styleSheets;for(var n=0;n<t.length;n++){if((t[n].ownerNode||t[n].owningElement)==$style[0]){stylesheet=t[n];break}}if(!stylesheet){throw new Error("Cannot find stylesheet.")}columnCssRulesL=[];columnCssRulesR=[];var r=stylesheet.cssRules||stylesheet.rules;var i,s;for(var n=0;n<r.length;n++){var o=r[n].selectorText;if(i=/\.l\d+/.exec(o)){s=parseInt(i[0].substr(2,i[0].length-2),10);columnCssRulesL[s]=r[n]}else if(i=/\.r\d+/.exec(o)){s=parseInt(i[0].substr(2,i[0].length-2),10);columnCssRulesR[s]=r[n]}}}return{left:columnCssRulesL[e],right:columnCssRulesR[e]}}function removeCssRules(){$style.remove();stylesheet=null}function destroy(){getEditorLock().cancelCurrentEdit();trigger(self.onBeforeDestroy,{});var e=plugins.length;while(e--){unregisterPlugin(plugins[e])}if(options.enableColumnReorder){$headers.filter(":ui-sortable").sortable("destroy")}unbindAncestorScrollEvents();$container.unbind(".slickgrid");removeCssRules();$canvas.unbind("draginit dragstart dragend drag");$container.empty().removeClass(uid)}function trigger(e,t,n){n=n||new Slick.EventData;t=t||{};t.grid=self;return e.notify(t,n,self)}function getEditorLock(){return options.editorLock}function getEditController(){return editController}function getColumnIndex(e){return columnsById[e]}function autosizeColumns(){var e,t,n=[],r=0,i=0,s,o=viewportHasVScroll?viewportW-scrollbarDimensions.width:viewportW;for(e=0;e<columns.length;e++){t=columns[e];n.push(t.width);i+=t.width;if(t.resizable){r+=t.width-Math.max(t.minWidth,absoluteColumnMinWidth)}}s=i;while(i>o&&r){var u=(i-o)/r;for(e=0;e<columns.length&&i>o;e++){t=columns[e];var a=n[e];if(!t.resizable||a<=t.minWidth||a<=absoluteColumnMinWidth){continue}var f=Math.max(t.minWidth,absoluteColumnMinWidth);var l=Math.floor(u*(a-f))||1;l=Math.min(l,a-f);i-=l;r-=l;n[e]-=l}if(s<=i){break}s=i}s=i;while(i<o){var c=o/i;for(e=0;e<columns.length&&i<o;e++){t=columns[e];var h=n[e];var p;if(!t.resizable||t.maxWidth<=h){p=0}else{p=Math.min(Math.floor(c*h)-h,t.maxWidth-h||1e6)||1}i+=p;n[e]+=p}if(s>=i){break}s=i}var d=false;for(e=0;e<columns.length;e++){if(columns[e].rerenderOnResize&&columns[e].width!=n[e]){d=true}columns[e].width=n[e]}applyColumnHeaderWidths();updateCanvasWidth(true);if(d){invalidateAllRows();render()}}function applyColumnHeaderWidths(){if(!initialized){return}var e;for(var t=0,n=$headers.children(),r=n.length;t<r;t++){e=$(n[t]);if(e.width()!==columns[t].width-headerColumnWidthDiff){e.width(columns[t].width-headerColumnWidthDiff)}}updateColumnCaches()}function applyColumnWidths(){var e=0,t,n;for(var r=0;r<columns.length;r++){t=columns[r].width;n=getColumnCssRules(r);n.left.style.left=e+"px";n.right.style.right=canvasWidth-e-t+"px";e+=columns[r].width}}function setSortColumn(e,t){setSortColumns([{columnId:e,sortAsc:t}])}function setSortColumns(e){sortColumns=e;var t=$headers.children();t.removeClass("slick-header-column-sorted").find(".slick-sort-indicator").removeClass("slick-sort-indicator-asc slick-sort-indicator-desc");$.each(sortColumns,function(e,n){if(n.sortAsc==null){n.sortAsc=true}var r=getColumnIndex(n.columnId);if(r!=null){t.eq(r).addClass("slick-header-column-sorted").find(".slick-sort-indicator").addClass(n.sortAsc?"slick-sort-indicator-asc":"slick-sort-indicator-desc")}})}function getSortColumns(){return sortColumns}function handleSelectedRangesChanged(e,t){selectedRows=[];var n={};for(var r=0;r<t.length;r++){for(var i=t[r].fromRow;i<=t[r].toRow;i++){if(!n[i]){selectedRows.push(i);n[i]={}}for(var s=t[r].fromCell;s<=t[r].toCell;s++){if(canCellBeSelected(i,s)){n[i][columns[s].id]=options.selectedCellCssClass}}}}setCellCssStyles(options.selectedCellCssClass,n);trigger(self.onSelectedRowsChanged,{rows:getSelectedRows()},e)}function getColumns(){return columns}function updateColumnCaches(){columnPosLeft=[];columnPosRight=[];var e=0;for(var t=0,n=columns.length;t<n;t++){columnPosLeft[t]=e;columnPosRight[t]=e+columns[t].width;e+=columns[t].width}}function setColumns(e){columns=e;columnsById={};for(var t=0;t<columns.length;t++){var n=columns[t]=$.extend({},columnDefaults,columns[t]);columnsById[n.id]=t;if(n.minWidth&&n.width<n.minWidth){n.width=n.minWidth}if(n.maxWidth&&n.width>n.maxWidth){n.width=n.maxWidth}}updateColumnCaches();if(initialized){invalidateAllRows();createColumnHeaders();removeCssRules();createCssRules();resizeCanvas();applyColumnWidths();handleScroll()}}function getOptions(){return options}function setOptions(e){if(!getEditorLock().commitCurrentEdit()){return}makeActiveCellNormal();if(options.enableAddRow!==e.enableAddRow){invalidateRow(getDataLength())}options=$.extend(options,e);validateAndEnforceOptions();$viewport.css("overflow-y",options.autoHeight?"hidden":"auto");render()}function validateAndEnforceOptions(){if(options.autoHeight){options.leaveSpaceForNewRows=false}}function setData(e,t){data=e;invalidateAllRows();updateRowCount();if(t){scrollTo(0)}}function getData(){return data}function getDataLength(){if(data.getLength){return data.getLength()}else{return data.length}}function getDataLengthIncludingAddNew(){return getDataLength()+(options.enableAddRow?1:0)}function getDataItem(e){if(data.getItem){return data.getItem(e)}else{return data[e]}}function getTopPanel(){return $topPanel[0]}function setTopPanelVisibility(e){if(options.showTopPanel!=e){options.showTopPanel=e;if(e){$topPanelScroller.slideDown("fast",resizeCanvas)}else{$topPanelScroller.slideUp("fast",resizeCanvas)}}}function setHeaderRowVisibility(e){if(options.showHeaderRow!=e){options.showHeaderRow=e;if(e){$headerRowScroller.slideDown("fast",resizeCanvas)}else{$headerRowScroller.slideUp("fast",resizeCanvas)}}}function getContainerNode(){return $container.get(0)}function getRowTop(e){return options.rowHeight*e-offset}function getRowFromPosition(e){return Math.floor((e+offset)/options.rowHeight)}function scrollTo(e){e=Math.max(e,0);e=Math.min(e,th-viewportH+(viewportHasHScroll?scrollbarDimensions.height:0));var t=offset;page=Math.min(n-1,Math.floor(e/ph));offset=Math.round(page*cj);var r=e-offset;if(offset!=t){var i=getVisibleRange(r);cleanupRows(i);updateRowPositions()}if(prevScrollTop!=r){vScrollDir=prevScrollTop+t<r+offset?1:-1;$viewport[0].scrollTop=lastRenderedScrollTop=scrollTop=prevScrollTop=r;trigger(self.onViewportChanged,{})}}function defaultFormatter(e,t,n,r,i){if(n==null){return""}else{return(n+"").replace(/&/g,"&").replace(/</g,"&lt;").replace(/>/g,"&gt;")}}function getFormatter(e,t){var n=data.getItemMetadata&&data.getItemMetadata(e);var r=n&&n.columns&&(n.columns[t.id]||n.columns[getColumnIndex(t.id)]);return r&&r.formatter||n&&n.formatter||t.formatter||options.formatterFactory&&options.formatterFactory.getFormatter(t)||options.defaultFormatter}function getEditor(e,t){var n=columns[t];var r=data.getItemMetadata&&data.getItemMetadata(e);var i=r&&r.columns;if(i&&i[n.id]&&i[n.id].editor!==undefined){return i[n.id].editor}if(i&&i[t]&&i[t].editor!==undefined){return i[t].editor}return n.editor||options.editorFactory&&options.editorFactory.getEditor(n)}function getDataItemValueForColumn(e,t){if(options.dataItemColumnValueExtractor){return options.dataItemColumnValueExtractor(e,t)}return e[t.field]}function appendRowHtml(e,t,n,r){var i=getDataItem(t);var s=t<r&&!i;var o="slick-row"+(s?" loading":"")+(t===activeRow?" active":"")+(t%2==1?" odd":" even");if(!i){o+=" "+options.addNewRowCssClass}var u=data.getItemMetadata&&data.getItemMetadata(t);if(u&&u.cssClasses){o+=" "+u.cssClasses}e.push("<div class='ui-widget-content "+o+"' style='top:"+getRowTop(t)+"px'>");var a,f;for(var l=0,c=columns.length;l<c;l++){f=columns[l];a=1;if(u&&u.columns){var h=u.columns[f.id]||u.columns[l];a=h&&h.colspan||1;if(a==="*"){a=c-l}}if(columnPosRight[Math.min(c-1,l+a-1)]>n.leftPx){if(columnPosLeft[l]>n.rightPx){break}appendCellHtml(e,t,l,a,i)}if(a>1){l+=a-1}}e.push("</div>")}function appendCellHtml(e,t,n,r,i){var s=columns[n];var o="slick-cell l"+n+" r"+Math.min(columns.length-1,n+r-1)+(s.cssClass?" "+s.cssClass:"");if(t===activeRow&&n===activeCell){o+=" active"}for(var u in cellCssClasses){if(cellCssClasses[u][t]&&cellCssClasses[u][t][s.id]){o+=" "+cellCssClasses[u][t][s.id]}}e.push("<div class='"+o+"'>");if(i){var a=getDataItemValueForColumn(i,s);e.push(getFormatter(t,s)(t,n,a,s,i))}e.push("</div>");rowsCache[t].cellRenderQueue.push(n);rowsCache[t].cellColSpans[n]=r}function cleanupRows(e){for(var t in rowsCache){if((t=parseInt(t,10))!==activeRow&&(t<e.top||t>e.bottom)){removeRowFromCache(t)}}}function invalidate(){updateRowCount();invalidateAllRows();render()}function invalidateAllRows(){if(currentEditor){makeActiveCellNormal()}for(var e in rowsCache){removeRowFromCache(e)}}function removeRowFromCache(e){var t=rowsCache[e];if(!t){return}if(rowNodeFromLastMouseWheelEvent==t.rowNode){t.rowNode.style.display="none";zombieRowNodeFromLastMouseWheelEvent=rowNodeFromLastMouseWheelEvent}else{$canvas[0].removeChild(t.rowNode)}delete rowsCache[e];delete postProcessedRows[e];renderedRows--;counter_rows_removed++}function invalidateRows(e){var t,n;if(!e||!e.length){return}vScrollDir=0;for(t=0,n=e.length;t<n;t++){if(currentEditor&&activeRow===e[t]){makeActiveCellNormal()}if(rowsCache[e[t]]){removeRowFromCache(e[t])}}}function invalidateRow(e){invalidateRows([e])}function updateCell(e,t){var n=getCellNode(e,t);if(!n){return}var r=columns[t],i=getDataItem(e);if(currentEditor&&activeRow===e&&activeCell===t){currentEditor.loadValue(i)}else{n.innerHTML=i?getFormatter(e,r)(e,t,getDataItemValueForColumn(i,r),r,i):"";invalidatePostProcessingResults(e)}}function updateRow(e){var t=rowsCache[e];if(!t){return}ensureCellNodesInRowsCache(e);var n=getDataItem(e);for(var r in t.cellNodesByColumnIdx){if(!t.cellNodesByColumnIdx.hasOwnProperty(r)){continue}r=r|0;var i=columns[r],s=t.cellNodesByColumnIdx[r];if(e===activeRow&&r===activeCell&&currentEditor){currentEditor.loadValue(n)}else if(n){s.innerHTML=getFormatter(e,i)(e,r,getDataItemValueForColumn(n,i),i,n)}else{s.innerHTML=""}}invalidatePostProcessingResults(e)}function getViewportHeight(){return parseFloat($.css($container[0],"height",true))-parseFloat($.css($container[0],"paddingTop",true))-parseFloat($.css($container[0],"paddingBottom",true))-parseFloat($.css($headerScroller[0],"height"))-getVBoxDelta($headerScroller)-(options.showTopPanel?options.topPanelHeight+getVBoxDelta($topPanelScroller):0)-(options.showHeaderRow?options.headerRowHeight+getVBoxDelta($headerRowScroller):0)}function resizeCanvas(){if(!initialized){return}if(options.autoHeight){viewportH=options.rowHeight*getDataLengthIncludingAddNew()}else{viewportH=getViewportHeight()}numVisibleRows=Math.ceil(viewportH/options.rowHeight);viewportW=parseFloat($.css($container[0],"width",true));if(!options.autoHeight){$viewport.height(viewportH)}if(options.forceFitColumns){autosizeColumns()}updateRowCount();handleScroll();lastRenderedScrollLeft=-1;render()}function updateRowCount(){if(!initialized){return}var e=getDataLengthIncludingAddNew();var t=e+(options.leaveSpaceForNewRows?numVisibleRows-1:0);var r=viewportHasVScroll;viewportHasVScroll=!options.autoHeight&&t*options.rowHeight>viewportH;makeActiveCellNormal();var i=e-1;for(var s in rowsCache){if(s>=i){removeRowFromCache(s)}}if(activeCellNode&&activeRow>i){resetActiveCell()}var o=h;th=Math.max(options.rowHeight*t,viewportH-scrollbarDimensions.height);if(th<maxSupportedCssHeight){h=ph=th;n=1;cj=0}else{h=maxSupportedCssHeight;ph=h/100;n=Math.floor(th/ph);cj=(th-h)/(n-1)}if(h!==o){$canvas.css("height",h);scrollTop=$viewport[0].scrollTop}var u=scrollTop+offset<=th-viewportH;if(th==0||scrollTop==0){page=offset=0}else if(u){scrollTo(scrollTop+offset)}else{scrollTo(th-viewportH)}if(h!=o&&options.autoHeight){resizeCanvas()}if(options.forceFitColumns&&r!=viewportHasVScroll){autosizeColumns()}updateCanvasWidth(false)}function getVisibleRange(e,t){if(e==null){e=scrollTop}if(t==null){t=scrollLeft}return{top:getRowFromPosition(e),bottom:getRowFromPosition(e+viewportH)+1,leftPx:t,rightPx:t+viewportW}}function getRenderedRange(e,t){var n=getVisibleRange(e,t);var r=Math.round(viewportH/options.rowHeight);var i=3;if(vScrollDir==-1){n.top-=r;n.bottom+=i}else if(vScrollDir==1){n.top-=i;n.bottom+=r}else{n.top-=i;n.bottom+=i}n.top=Math.max(0,n.top);n.bottom=Math.min(getDataLengthIncludingAddNew()-1,n.bottom);n.leftPx-=viewportW;n.rightPx+=viewportW;n.leftPx=Math.max(0,n.leftPx);n.rightPx=Math.min(canvasWidth,n.rightPx);return n}function ensureCellNodesInRowsCache(e){var t=rowsCache[e];if(t){if(t.cellRenderQueue.length){var n=t.rowNode.lastChild;while(t.cellRenderQueue.length){var r=t.cellRenderQueue.pop();t.cellNodesByColumnIdx[r]=n;n=n.previousSibling}}}}function cleanUpCells(e,t){var n=0;var r=rowsCache[t];var i=[];for(var s in r.cellNodesByColumnIdx){if(!r.cellNodesByColumnIdx.hasOwnProperty(s)){continue}s=s|0;var o=r.cellColSpans[s];if(columnPosLeft[s]>e.rightPx||columnPosRight[Math.min(columns.length-1,s+o-1)]<e.leftPx){if(!(t==activeRow&&s==activeCell)){i.push(s)}}}var u;while((u=i.pop())!=null){r.rowNode.removeChild(r.cellNodesByColumnIdx[u]);delete r.cellColSpans[u];delete r.cellNodesByColumnIdx[u];if(postProcessedRows[t]){delete postProcessedRows[t][u]}n++}}function cleanUpAndRenderCells(e){var t;var n=[];var r=[];var i;var s=0;var o;for(var u=e.top,a=e.bottom;u<=a;u++){t=rowsCache[u];if(!t){continue}ensureCellNodesInRowsCache(u);cleanUpCells(e,u);i=0;var f=data.getItemMetadata&&data.getItemMetadata(u);f=f&&f.columns;var l=getDataItem(u);for(var c=0,h=columns.length;c<h;c++){if(columnPosLeft[c]>e.rightPx){break}if((o=t.cellColSpans[c])!=null){c+=o>1?o-1:0;continue}o=1;if(f){var p=f[columns[c].id]||f[c];o=p&&p.colspan||1;if(o==="*"){o=h-c}}if(columnPosRight[Math.min(h-1,c+o-1)]>e.leftPx){appendCellHtml(n,u,c,o,l);i++}c+=o>1?o-1:0}if(i){s+=i;r.push(u)}}if(!n.length){return}var d=document.createElement("div");d.innerHTML=n.join("");var v;var m;while((v=r.pop())!=null){t=rowsCache[v];var g;while((g=t.cellRenderQueue.pop())!=null){m=d.lastChild;t.rowNode.appendChild(m);t.cellNodesByColumnIdx[g]=m}}}function renderRows(e){var t=$canvas[0],n=[],r=[],i=false,s=getDataLength();for(var o=e.top,u=e.bottom;o<=u;o++){if(rowsCache[o]){continue}renderedRows++;r.push(o);rowsCache[o]={rowNode:null,cellColSpans:[],cellNodesByColumnIdx:[],cellRenderQueue:[]};appendRowHtml(n,o,e,s);if(activeCellNode&&activeRow===o){i=true}counter_rows_rendered++}if(!r.length){return}var a=document.createElement("div");a.innerHTML=n.join("");for(var o=0,u=r.length;o<u;o++){rowsCache[r[o]].rowNode=t.appendChild(a.firstChild)}if(i){activeCellNode=getCellNode(activeRow,activeCell)}}function startPostProcessing(){if(!options.enableAsyncPostRender){return}clearTimeout(h_postrender);h_postrender=setTimeout(asyncPostProcessRows,options.asyncPostRenderDelay)}function invalidatePostProcessingResults(e){delete postProcessedRows[e];postProcessFromRow=Math.min(postProcessFromRow,e);postProcessToRow=Math.max(postProcessToRow,e);startPostProcessing()}function updateRowPositions(){for(var e in rowsCache){rowsCache[e].rowNode.style.top=getRowTop(e)+"px"}}function render(){if(!initialized){return}var e=getVisibleRange();var t=getRenderedRange();cleanupRows(t);if(lastRenderedScrollLeft!=scrollLeft){cleanUpAndRenderCells(t)}renderRows(t);postProcessFromRow=e.top;postProcessToRow=Math.min(getDataLengthIncludingAddNew()-1,e.bottom);startPostProcessing();lastRenderedScrollTop=scrollTop;lastRenderedScrollLeft=scrollLeft;h_render=null}function handleHeaderRowScroll(){var e=$headerRowScroller[0].scrollLeft;if(e!=$viewport[0].scrollLeft){$viewport[0].scrollLeft=e}}function handleScroll(){scrollTop=$viewport[0].scrollTop;scrollLeft=$viewport[0].scrollLeft;var e=Math.abs(scrollTop-prevScrollTop);var t=Math.abs(scrollLeft-prevScrollLeft);if(t){prevScrollLeft=scrollLeft;$headerScroller[0].scrollLeft=scrollLeft;$topPanelScroller[0].scrollLeft=scrollLeft;$headerRowScroller[0].scrollLeft=scrollLeft}if(e){vScrollDir=prevScrollTop<scrollTop?1:-1;prevScrollTop=scrollTop;if(e<viewportH){scrollTo(scrollTop+offset)}else{var r=offset;if(h==viewportH){page=0}else{page=Math.min(n-1,Math.floor(scrollTop*((th-viewportH)/(h-viewportH))*(1/ph)))}offset=Math.round(page*cj);if(r!=offset){invalidateAllRows()}}}if(t||e){if(h_render){clearTimeout(h_render)}if(Math.abs(lastRenderedScrollTop-scrollTop)>20||Math.abs(lastRenderedScrollLeft-scrollLeft)>20){if(options.forceSyncScrolling||Math.abs(lastRenderedScrollTop-scrollTop)<viewportH&&Math.abs(lastRenderedScrollLeft-scrollLeft)<viewportW){render()}else{h_render=setTimeout(render,50)}trigger(self.onViewportChanged,{})}}trigger(self.onScroll,{scrollLeft:scrollLeft,scrollTop:scrollTop})}function asyncPostProcessRows(){var e=getDataLength();while(postProcessFromRow<=postProcessToRow){var t=vScrollDir>=0?postProcessFromRow++:postProcessToRow--;var n=rowsCache[t];if(!n||t>=e){continue}if(!postProcessedRows[t]){postProcessedRows[t]={}}ensureCellNodesInRowsCache(t);for(var r in n.cellNodesByColumnIdx){if(!n.cellNodesByColumnIdx.hasOwnProperty(r)){continue}r=r|0;var i=columns[r];if(i.asyncPostRender&&!postProcessedRows[t][r]){var s=n.cellNodesByColumnIdx[r];if(s){i.asyncPostRender(s,t,getDataItem(t),i)}postProcessedRows[t][r]=true}}h_postrender=setTimeout(asyncPostProcessRows,options.asyncPostRenderDelay);return}}function updateCellCssStylesOnRenderedRows(e,t){var n,r,i,s;for(var o in rowsCache){s=t&&t[o];i=e&&e[o];if(s){for(r in s){if(!i||s[r]!=i[r]){n=getCellNode(o,getColumnIndex(r));if(n){$(n).removeClass(s[r])}}}}if(i){for(r in i){if(!s||s[r]!=i[r]){n=getCellNode(o,getColumnIndex(r));if(n){$(n).addClass(i[r])}}}}}}function addCellCssStyles(e,t){if(cellCssClasses[e]){throw"addCellCssStyles: cell CSS hash with key '"+e+"' already exists."}cellCssClasses[e]=t;updateCellCssStylesOnRenderedRows(t,null);trigger(self.onCellCssStylesChanged,{key:e,hash:t})}function removeCellCssStyles(e){if(!cellCssClasses[e]){return}updateCellCssStylesOnRenderedRows(null,cellCssClasses[e]);delete cellCssClasses[e];trigger(self.onCellCssStylesChanged,{key:e,hash:null})}function setCellCssStyles(e,t){var n=cellCssClasses[e];cellCssClasses[e]=t;updateCellCssStylesOnRenderedRows(t,n);trigger(self.onCellCssStylesChanged,{key:e,hash:t})}function getCellCssStyles(e){return cellCssClasses[e]}function flashCell(e,t,n){n=n||100;if(rowsCache[e]){var r=$(getCellNode(e,t));function i(e){if(!e){return}setTimeout(function(){r.queue(function(){r.toggleClass(options.cellFlashingCssClass).dequeue();i(e-1)})},n)}i(4)}}function handleMouseWheel(e){var t=$(e.target).closest(".slick-row")[0];if(t!=rowNodeFromLastMouseWheelEvent){if(zombieRowNodeFromLastMouseWheelEvent&&zombieRowNodeFromLastMouseWheelEvent!=t){$canvas[0].removeChild(zombieRowNodeFromLastMouseWheelEvent);zombieRowNodeFromLastMouseWheelEvent=null}rowNodeFromLastMouseWheelEvent=t}}function handleDragInit(e,t){var n=getCellFromEvent(e);if(!n||!cellExists(n.row,n.cell)){return false}var r=trigger(self.onDragInit,t,e);if(e.isImmediatePropagationStopped()){return r}return false}function handleDragStart(e,t){var n=getCellFromEvent(e);if(!n||!cellExists(n.row,n.cell)){return false}var r=trigger(self.onDragStart,t,e);if(e.isImmediatePropagationStopped()){return r}return false}function handleDrag(e,t){return trigger(self.onDrag,t,e)}function handleDragEnd(e,t){trigger(self.onDragEnd,t,e)}function handleKeyDown(e){trigger(self.onKeyDown,{row:activeRow,cell:activeCell},e);var t=e.isImmediatePropagationStopped();if(!t){if(!e.shiftKey&&!e.altKey&&!e.ctrlKey){if(e.which==27){if(!getEditorLock().isActive()){return}cancelEditAndSetFocus()}else if(e.which==34){navigatePageDown();t=true}else if(e.which==33){navigatePageUp();t=true}else if(e.which==37){t=navigateLeft()}else if(e.which==39){t=navigateRight()}else if(e.which==38){t=navigateUp()}else if(e.which==40){t=navigateDown()}else if(e.which==9){t=navigateNext()}else if(e.which==13){if(options.editable){if(currentEditor){if(activeRow===getDataLength()){navigateDown()}else{commitEditAndSetFocus()}}else{if(getEditorLock().commitCurrentEdit()){makeActiveCellEditable()}}}t=true}}else if(e.which==9&&e.shiftKey&&!e.ctrlKey&&!e.altKey){t=navigatePrev()}}if(t){e.stopPropagation();e.preventDefault();try{e.originalEvent.keyCode=0}catch(n){}}}function handleClick(e){if(!currentEditor){if(e.target!=document.activeElement||$(e.target).hasClass("slick-cell")){setFocus()}}var t=getCellFromEvent(e);if(!t||currentEditor!==null&&activeRow==t.row&&activeCell==t.cell){return}trigger(self.onClick,{row:t.row,cell:t.cell},e);if(e.isImmediatePropagationStopped()){return}if((activeCell!=t.cell||activeRow!=t.row)&&canCellBeActive(t.row,t.cell)){if(!getEditorLock().isActive()||getEditorLock().commitCurrentEdit()){scrollRowIntoView(t.row,false);setActiveCellInternal(getCellNode(t.row,t.cell))}}}function handleContextMenu(e){var t=$(e.target).closest(".slick-cell",$canvas);if(t.length===0){return}if(activeCellNode===t[0]&&currentEditor!==null){return}trigger(self.onContextMenu,{},e)}function handleDblClick(e){var t=getCellFromEvent(e);if(!t||currentEditor!==null&&activeRow==t.row&&activeCell==t.cell){return}trigger(self.onDblClick,{row:t.row,cell:t.cell},e);if(e.isImmediatePropagationStopped()){return}if(options.editable){gotoCell(t.row,t.cell,true)}}function handleHeaderMouseEnter(e){trigger(self.onHeaderMouseEnter,{column:$(this).data("column")},e)}function handleHeaderMouseLeave(e){trigger(self.onHeaderMouseLeave,{column:$(this).data("column")},e)}function handleHeaderContextMenu(e){var t=$(e.target).closest(".slick-header-column",".slick-header-columns");var n=t&&t.data("column");trigger(self.onHeaderContextMenu,{column:n},e)}function handleHeaderClick(e){var t=$(e.target).closest(".slick-header-column",".slick-header-columns");var n=t&&t.data("column");if(n){trigger(self.onHeaderClick,{column:n},e)}}function handleMouseEnter(e){trigger(self.onMouseEnter,{},e)}function handleMouseLeave(e){trigger(self.onMouseLeave,{},e)}function cellExists(e,t){return!(e<0||e>=getDataLength()||t<0||t>=columns.length)}function getCellFromPoint(e,t){var n=getRowFromPosition(t);var r=0;var i=0;for(var s=0;s<columns.length&&i<e;s++){i+=columns[s].width;r++}if(r<0){r=0}return{row:n,cell:r-1}}function getCellFromNode(e){var t=/l\d+/.exec(e.className);if(!t){throw"getCellFromNode: cannot get cell - "+e.className}return parseInt(t[0].substr(1,t[0].length-1),10)}function getRowFromNode(e){for(var t in rowsCache){if(rowsCache[t].rowNode===e){return t|0}}return null}function getCellFromEvent(e){var t=$(e.target).closest(".slick-cell",$canvas);if(!t.length){return null}var n=getRowFromNode(t[0].parentNode);var r=getCellFromNode(t[0]);if(n==null||r==null){return null}else{return{row:n,cell:r}}}function getCellNodeBox(e,t){if(!cellExists(e,t)){return null}var n=getRowTop(e);var r=n+options.rowHeight-1;var i=0;for(var s=0;s<t;s++){i+=columns[s].width}var o=i+columns[t].width;return{top:n,left:i,bottom:r,right:o}}function resetActiveCell(){setActiveCellInternal(null,false)}function setFocus(){if(tabbingDirection==-1){$focusSink[0].focus()}else{$focusSink2[0].focus()}}function scrollCellIntoView(e,t,n){scrollRowIntoView(e,n);var r=getColspan(e,t);var i=columnPosLeft[t],s=columnPosRight[t+(r>1?r-1:0)],o=scrollLeft+viewportW;if(i<scrollLeft){$viewport.scrollLeft(i);handleScroll();render()}else if(s>o){$viewport.scrollLeft(Math.min(i,s-$viewport[0].clientWidth));handleScroll();render()}}function setActiveCellInternal(e,t){if(activeCellNode!==null){makeActiveCellNormal();$(activeCellNode).removeClass("active");if(rowsCache[activeRow]){$(rowsCache[activeRow].rowNode).removeClass("active")}}var n=activeCellNode!==e;activeCellNode=e;if(activeCellNode!=null){activeRow=getRowFromNode(activeCellNode.parentNode);activeCell=activePosX=getCellFromNode(activeCellNode);if(t==null){t=activeRow==getDataLength()||options.autoEdit}$(activeCellNode).addClass("active");$(rowsCache[activeRow].rowNode).addClass("active");if(options.editable&&t&&isCellPotentiallyEditable(activeRow,activeCell)){clearTimeout(h_editorLoader);if(options.asyncEditorLoading){h_editorLoader=setTimeout(function(){makeActiveCellEditable()},options.asyncEditorLoadDelay)}else{makeActiveCellEditable()}}}else{activeRow=activeCell=null}if(n){trigger(self.onActiveCellChanged,getActiveCell())}}function clearTextSelection(){if(document.selection&&document.selection.empty){try{document.selection.empty()}catch(e){}}else if(window.getSelection){var t=window.getSelection();if(t&&t.removeAllRanges){t.removeAllRanges()}}}function isCellPotentiallyEditable(e,t){var n=getDataLength();if(e<n&&!getDataItem(e)){return false}if(columns[t].cannotTriggerInsert&&e>=n){return false}if(!getEditor(e,t)){return false}return true}function makeActiveCellNormal(){if(!currentEditor){return}trigger(self.onBeforeCellEditorDestroy,{editor:currentEditor});currentEditor.destroy();currentEditor=null;if(activeCellNode){var e=getDataItem(activeRow);$(activeCellNode).removeClass("editable invalid");if(e){var t=columns[activeCell];var n=getFormatter(activeRow,t);activeCellNode.innerHTML=n(activeRow,activeCell,getDataItemValueForColumn(e,t),t,e);invalidatePostProcessingResults(activeRow)}}if(navigator.userAgent.toLowerCase().match(/msie/)){clearTextSelection()}getEditorLock().deactivate(editController)}function makeActiveCellEditable(e){if(!activeCellNode){return}if(!options.editable){throw"Grid : makeActiveCellEditable : should never get called when options.editable is false"}clearTimeout(h_editorLoader);if(!isCellPotentiallyEditable(activeRow,activeCell)){return}var t=columns[activeCell];var n=getDataItem(activeRow);if(trigger(self.onBeforeEditCell,{row:activeRow,cell:activeCell,item:n,column:t})===false){setFocus();return}getEditorLock().activate(editController);$(activeCellNode).addClass("editable");if(!e){activeCellNode.innerHTML=""}currentEditor=new(e||getEditor(activeRow,activeCell))({grid:self,gridPosition:absBox($container[0]),position:absBox(activeCellNode),container:activeCellNode,column:t,item:n||{},commitChanges:commitEditAndSetFocus,cancelChanges:cancelEditAndSetFocus});if(n){currentEditor.loadValue(n)}serializedEditorValue=currentEditor.serializeValue();if(currentEditor.position){handleActiveCellPositionChange()}}function commitEditAndSetFocus(){if(getEditorLock().commitCurrentEdit()){setFocus();if(options.autoEdit){navigateDown()}}}function cancelEditAndSetFocus(){if(getEditorLock().cancelCurrentEdit()){setFocus()}}function absBox(e){var t={top:e.offsetTop,left:e.offsetLeft,bottom:0,right:0,width:$(e).outerWidth(),height:$(e).outerHeight(),visible:true};t.bottom=t.top+t.height;t.right=t.left+t.width;var n=e.offsetParent;while((e=e.parentNode)!=document.body){if(t.visible&&e.scrollHeight!=e.offsetHeight&&$(e).css("overflowY")!="visible"){t.visible=t.bottom>e.scrollTop&&t.top<e.scrollTop+e.clientHeight}if(t.visible&&e.scrollWidth!=e.offsetWidth&&$(e).css("overflowX")!="visible"){t.visible=t.right>e.scrollLeft&&t.left<e.scrollLeft+e.clientWidth}t.left-=e.scrollLeft;t.top-=e.scrollTop;if(e===n){t.left+=e.offsetLeft;t.top+=e.offsetTop;n=e.offsetParent}t.bottom=t.top+t.height;t.right=t.left+t.width}return t}function getActiveCellPosition(){return absBox(activeCellNode)}function getGridPosition(){return absBox($container[0])}function handleActiveCellPositionChange(){if(!activeCellNode){return}trigger(self.onActiveCellPositionChanged,{});if(currentEditor){var e=getActiveCellPosition();if(currentEditor.show&&currentEditor.hide){if(!e.visible){currentEditor.hide()}else{currentEditor.show()}}if(currentEditor.position){currentEditor.position(e)}}}function getCellEditor(){return currentEditor}function getActiveCell(){if(!activeCellNode){return null}else{return{row:activeRow,cell:activeCell}}}function getActiveCellNode(){return activeCellNode}function scrollRowIntoView(e,t){var n=e*options.rowHeight;var r=(e+1)*options.rowHeight-viewportH+(viewportHasHScroll?scrollbarDimensions.height:0);if((e+1)*options.rowHeight>scrollTop+viewportH+offset){scrollTo(t?n:r);render()}else if(e*options.rowHeight<scrollTop+offset){scrollTo(t?r:n);render()}}function scrollRowToTop(e){scrollTo(e*options.rowHeight);render()}function scrollPage(e){var t=e*numVisibleRows;scrollTo((getRowFromPosition(scrollTop)+t)*options.rowHeight);render();if(options.enableCellNavigation&&activeRow!=null){var n=activeRow+t;var r=getDataLengthIncludingAddNew();if(n>=r){n=r-1}if(n<0){n=0}var i=0,s=null;var o=activePosX;while(i<=activePosX){if(canCellBeActive(n,i)){s=i}i+=getColspan(n,i)}if(s!==null){setActiveCellInternal(getCellNode(n,s));activePosX=o}else{resetActiveCell()}}}function navigatePageDown(){scrollPage(1)}function navigatePageUp(){scrollPage(-1)}function getColspan(e,t){var n=data.getItemMetadata&&data.getItemMetadata(e);if(!n||!n.columns){return 1}var r=n.columns[columns[t].id]||n.columns[t];var i=r&&r.colspan;if(i==="*"){i=columns.length-t}else{i=i||1}return i}function findFirstFocusableCell(e){var t=0;while(t<columns.length){if(canCellBeActive(e,t)){return t}t+=getColspan(e,t)}return null}function findLastFocusableCell(e){var t=0;var n=null;while(t<columns.length){if(canCellBeActive(e,t)){n=t}t+=getColspan(e,t)}return n}function gotoRight(e,t,n){if(t>=columns.length){return null}do{t+=getColspan(e,t)}while(t<columns.length&&!canCellBeActive(e,t));if(t<columns.length){return{row:e,cell:t,posX:t}}return null}function gotoLeft(e,t,n){if(t<=0){return null}var r=findFirstFocusableCell(e);if(r===null||r>=t){return null}var i={row:e,cell:r,posX:r};var s;while(true){s=gotoRight(i.row,i.cell,i.posX);if(!s){return null}if(s.cell>=t){return i}i=s}}function gotoDown(e,t,n){var r;var i=getDataLengthIncludingAddNew();while(true){if(++e>=i){return null}r=t=0;while(t<=n){r=t;t+=getColspan(e,t)}if(canCellBeActive(e,r)){return{row:e,cell:r,posX:n}}}}function gotoUp(e,t,n){var r;while(true){if(--e<0){return null}r=t=0;while(t<=n){r=t;t+=getColspan(e,t)}if(canCellBeActive(e,r)){return{row:e,cell:r,posX:n}}}}function gotoNext(e,t,n){if(e==null&&t==null){e=t=n=0;if(canCellBeActive(e,t)){return{row:e,cell:t,posX:t}}}var r=gotoRight(e,t,n);if(r){return r}var i=null;var s=getDataLengthIncludingAddNew();while(++e<s){i=findFirstFocusableCell(e);if(i!==null){return{row:e,cell:i,posX:i}}}return null}function gotoPrev(e,t,n){if(e==null&&t==null){e=getDataLengthIncludingAddNew()-1;t=n=columns.length-1;if(canCellBeActive(e,t)){return{row:e,cell:t,posX:t}}}var r;var i;while(!r){r=gotoLeft(e,t,n);if(r){break}if(--e<0){return null}t=0;i=findLastFocusableCell(e);if(i!==null){r={row:e,cell:i,posX:i}}}return r}function navigateRight(){return navigate("right")}function navigateLeft(){return navigate("left")}function navigateDown(){return navigate("down")}function navigateUp(){return navigate("up")}function navigateNext(){return navigate("next")}function navigatePrev(){return navigate("prev")}function navigate(e){if(!options.enableCellNavigation){return false}if(!activeCellNode&&e!="prev"&&e!="next"){return false}if(!getEditorLock().commitCurrentEdit()){return true}setFocus();var t={up:-1,down:1,left:-1,right:1,prev:-1,next:1};tabbingDirection=t[e];var n={up:gotoUp,down:gotoDown,left:gotoLeft,right:gotoRight,prev:gotoPrev,next:gotoNext};var r=n[e];var i=r(activeRow,activeCell,activePosX);if(i){var s=i.row==getDataLength();scrollCellIntoView(i.row,i.cell,!s);setActiveCellInternal(getCellNode(i.row,i.cell));activePosX=i.posX;return true}else{setActiveCellInternal(getCellNode(activeRow,activeCell));return false}}function getCellNode(e,t){if(rowsCache[e]){ensureCellNodesInRowsCache(e);return rowsCache[e].cellNodesByColumnIdx[t]}return null}function setActiveCell(e,t){if(!initialized){return}if(e>getDataLength()||e<0||t>=columns.length||t<0){return}if(!options.enableCellNavigation){return}scrollCellIntoView(e,t,false);setActiveCellInternal(getCellNode(e,t),false)}function canCellBeActive(e,t){if(!options.enableCellNavigation||e>=getDataLengthIncludingAddNew()||e<0||t>=columns.length||t<0){return false}var n=data.getItemMetadata&&data.getItemMetadata(e);if(n&&typeof n.focusable==="boolean"){return n.focusable}var r=n&&n.columns;if(r&&r[columns[t].id]&&typeof r[columns[t].id].focusable==="boolean"){return r[columns[t].id].focusable}if(r&&r[t]&&typeof r[t].focusable==="boolean"){return r[t].focusable}return columns[t].focusable}function canCellBeSelected(e,t){if(e>=getDataLength()||e<0||t>=columns.length||t<0){return false}var n=data.getItemMetadata&&data.getItemMetadata(e);if(n&&typeof n.selectable==="boolean"){return n.selectable}var r=n&&n.columns&&(n.columns[columns[t].id]||n.columns[t]);if(r&&typeof r.selectable==="boolean"){return r.selectable}return columns[t].selectable}function gotoCell(e,t,n){if(!initialized){return}if(!canCellBeActive(e,t)){return}if(!getEditorLock().commitCurrentEdit()){return}scrollCellIntoView(e,t,false);var r=getCellNode(e,t);setActiveCellInternal(r,n||e===getDataLength()||options.autoEdit);if(!currentEditor){setFocus()}}function commitCurrentEdit(){var e=getDataItem(activeRow);var t=columns[activeCell];if(currentEditor){if(currentEditor.isValueChanged()){var n=currentEditor.validate();if(n.valid){if(activeRow<getDataLength()){var r={row:activeRow,cell:activeCell,editor:currentEditor,serializedValue:currentEditor.serializeValue(),prevSerializedValue:serializedEditorValue,execute:function(){this.editor.applyValue(e,this.serializedValue);updateRow(this.row);trigger(self.onCellChange,{row:activeRow,cell:activeCell,item:e})},undo:function(){this.editor.applyValue(e,this.prevSerializedValue);updateRow(this.row);trigger(self.onCellChange,{row:activeRow,cell:activeCell,item:e})}};if(options.editCommandHandler){makeActiveCellNormal();options.editCommandHandler(e,t,r)}else{r.execute();makeActiveCellNormal()}}else{var i={};currentEditor.applyValue(i,currentEditor.serializeValue());makeActiveCellNormal();trigger(self.onAddNewRow,{item:i,column:t})}return!getEditorLock().isActive()}else{$(activeCellNode).removeClass("invalid");$(activeCellNode).width();$(activeCellNode).addClass("invalid");trigger(self.onValidationError,{editor:currentEditor,cellNode:activeCellNode,validationResults:n,row:activeRow,cell:activeCell,column:t});currentEditor.focus();return false}}makeActiveCellNormal()}return true}function cancelCurrentEdit(){makeActiveCellNormal();return true}function rowsToRanges(e){var t=[];var n=columns.length-1;for(var r=0;r<e.length;r++){t.push(new Slick.Range(e[r],0,e[r],n))}return t}function getSelectedRows(){if(!selectionModel){throw"Selection model is not set"}return selectedRows}function setSelectedRows(e){if(!selectionModel){throw"Selection model is not set"}selectionModel.setSelectedRanges(rowsToRanges(e))}var defaults={explicitInitialization:false,rowHeight:25,defaultColumnWidth:80,enableAddRow:false,leaveSpaceForNewRows:false,editable:false,autoEdit:true,enableCellNavigation:true,enableColumnReorder:true,asyncEditorLoading:false,asyncEditorLoadDelay:100,forceFitColumns:false,enableAsyncPostRender:false,asyncPostRenderDelay:50,autoHeight:false,editorLock:Slick.GlobalEditorLock,showHeaderRow:false,headerRowHeight:25,showTopPanel:false,topPanelHeight:25,formatterFactory:null,editorFactory:null,cellFlashingCssClass:"flashing",selectedCellCssClass:"selected",multiSelect:true,enableTextSelectionOnCells:false,dataItemColumnValueExtractor:null,fullWidthRows:false,multiColumnSort:false,defaultFormatter:defaultFormatter,forceSyncScrolling:false,addNewRowCssClass:"new-row"};var columnDefaults={name:"",resizable:true,sortable:false,minWidth:30,rerenderOnResize:false,headerCssClass:null,defaultSortAsc:true,focusable:true,selectable:true};var th;var h;var ph;var n;var cj;var page=0;var offset=0;var vScrollDir=1;var initialized=false;var $container;var uid="slickgrid_"+Math.round(1e6*Math.random());var self=this;var $focusSink,$focusSink2;var $headerScroller;var $headers;var $headerRow,$headerRowScroller,$headerRowSpacer;var $topPanelScroller;var $topPanel;var $viewport;var $canvas;var $style;var $boundAncestors;var stylesheet,columnCssRulesL,columnCssRulesR;var viewportH,viewportW;var canvasWidth;var viewportHasHScroll,viewportHasVScroll;var headerColumnWidthDiff=0,headerColumnHeightDiff=0,cellWidthDiff=0,cellHeightDiff=0;var absoluteColumnMinWidth;var tabbingDirection=1;var activePosX;var activeRow,activeCell;var activeCellNode=null;var currentEditor=null;var serializedEditorValue;var editController;var rowsCache={};var renderedRows=0;var numVisibleRows;var prevScrollTop=0;var scrollTop=0;var lastRenderedScrollTop=0;var lastRenderedScrollLeft=0;var prevScrollLeft=0;var scrollLeft=0;var selectionModel;var selectedRows=[];var plugins=[];var cellCssClasses={};var columnsById={};var sortColumns=[];var columnPosLeft=[];var columnPosRight=[];var h_editorLoader=null;var h_render=null;var h_postrender=null;var postProcessedRows={};var postProcessToRow=null;var postProcessFromRow=null;var counter_rows_rendered=0;var counter_rows_removed=0;var rowNodeFromLastMouseWheelEvent;var zombieRowNodeFromLastMouseWheelEvent;this.debug=function(){var e="";e+="\n"+"counter_rows_rendered:  "+counter_rows_rendered;e+="\n"+"counter_rows_removed:  "+counter_rows_removed;e+="\n"+"renderedRows:  "+renderedRows;e+="\n"+"numVisibleRows:  "+numVisibleRows;e+="\n"+"maxSupportedCssHeight:  "+maxSupportedCssHeight;e+="\n"+"n(umber of pages):  "+n;e+="\n"+"(current) page:  "+page;e+="\n"+"page height (ph):  "+ph;e+="\n"+"vScrollDir:  "+vScrollDir;alert(e)};this.eval=function(expr){return eval(expr)};$.extend(this,{slickGridVersion:"2.1",onScroll:new Slick.Event,onSort:new Slick.Event,onHeaderMouseEnter:new Slick.Event,onHeaderMouseLeave:new Slick.Event,onHeaderContextMenu:new Slick.Event,onHeaderClick:new Slick.Event,onHeaderCellRendered:new Slick.Event,onBeforeHeaderCellDestroy:new Slick.Event,onHeaderRowCellRendered:new Slick.Event,onBeforeHeaderRowCellDestroy:new Slick.Event,onMouseEnter:new Slick.Event,onMouseLeave:new Slick.Event,onClick:new Slick.Event,onDblClick:new Slick.Event,onContextMenu:new Slick.Event,onKeyDown:new Slick.Event,onAddNewRow:new Slick.Event,onValidationError:new Slick.Event,onViewportChanged:new Slick.Event,onColumnsReordered:new Slick.Event,onColumnsResized:new Slick.Event,onCellChange:new Slick.Event,onBeforeEditCell:new Slick.Event,onBeforeCellEditorDestroy:new Slick.Event,onBeforeDestroy:new Slick.Event,onActiveCellChanged:new Slick.Event,onActiveCellPositionChanged:new Slick.Event,onDragInit:new Slick.Event,onDragStart:new Slick.Event,onDrag:new Slick.Event,onDragEnd:new Slick.Event,onSelectedRowsChanged:new Slick.Event,onCellCssStylesChanged:new Slick.Event,registerPlugin:registerPlugin,unregisterPlugin:unregisterPlugin,getColumns:getColumns,setColumns:setColumns,getColumnIndex:getColumnIndex,updateColumnHeader:updateColumnHeader,setSortColumn:setSortColumn,setSortColumns:setSortColumns,getSortColumns:getSortColumns,autosizeColumns:autosizeColumns,getOptions:getOptions,setOptions:setOptions,getData:getData,getDataLength:getDataLength,getDataItem:getDataItem,setData:setData,getSelectionModel:getSelectionModel,setSelectionModel:setSelectionModel,getSelectedRows:getSelectedRows,setSelectedRows:setSelectedRows,getContainerNode:getContainerNode,render:render,invalidate:invalidate,invalidateRow:invalidateRow,invalidateRows:invalidateRows,invalidateAllRows:invalidateAllRows,updateCell:updateCell,updateRow:updateRow,getViewport:getVisibleRange,getRenderedRange:getRenderedRange,resizeCanvas:resizeCanvas,updateRowCount:updateRowCount,scrollRowIntoView:scrollRowIntoView,scrollRowToTop:scrollRowToTop,scrollCellIntoView:scrollCellIntoView,getCanvasNode:getCanvasNode,focus:setFocus,getCellFromPoint:getCellFromPoint,getCellFromEvent:getCellFromEvent,getActiveCell:getActiveCell,setActiveCell:setActiveCell,getActiveCellNode:getActiveCellNode,getActiveCellPosition:getActiveCellPosition,resetActiveCell:resetActiveCell,editActiveCell:makeActiveCellEditable,getCellEditor:getCellEditor,getCellNode:getCellNode,getCellNodeBox:getCellNodeBox,canCellBeSelected:canCellBeSelected,canCellBeActive:canCellBeActive,navigatePrev:navigatePrev,navigateNext:navigateNext,navigateUp:navigateUp,navigateDown:navigateDown,navigateLeft:navigateLeft,navigateRight:navigateRight,navigatePageUp:navigatePageUp,navigatePageDown:navigatePageDown,gotoCell:gotoCell,getTopPanel:getTopPanel,setTopPanelVisibility:setTopPanelVisibility,setHeaderRowVisibility:setHeaderRowVisibility,getHeaderRow:getHeaderRow,getHeaderRowColumn:getHeaderRowColumn,getGridPosition:getGridPosition,flashCell:flashCell,addCellCssStyles:addCellCssStyles,setCellCssStyles:setCellCssStyles,removeCellCssStyles:removeCellCssStyles,getCellCssStyles:getCellCssStyles,init:finishInitialization,destroy:destroy,getEditorLock:getEditorLock,getEditController:getEditController});init()}$.extend(true,window,{Slick:{Grid:SlickGrid}});var scrollbarDimensions;var maxSupportedCssHeight})(jQuery);(function(e){function t(t){function D(){c=true}function P(){c=false;Dt()}function H(e){v=e}function B(e){g=e}function j(e){e=e||0;var t;for(var n=e,r=s.length;n<r;n++){t=s[n][i];if(t===undefined){throw"Each data element must implement a unique 'id' property"}u[t]=n}}function F(){var e;for(var t=0,n=s.length;t<n;t++){e=s[t][i];if(e===undefined||u[e]!==t){throw"Each data element must implement a unique 'id' property"}}}function I(){return s}function q(e,t){if(t!==undefined){i=t}s=y=e;u={};j();F();Dt()}function R(e){if(e.pageSize!=undefined){k=e.pageSize;L=k?Math.min(L,Math.max(0,Math.ceil(A/k)-1)):0}if(e.pageNum!=undefined){L=Math.min(e.pageNum,Math.max(0,Math.ceil(A/k)-1))}_.notify(U(),null,n);Dt()}function U(){var e=k?Math.max(1,Math.ceil(A/k)):1;return{pageSize:k,pageNum:L,totalRows:A,totalPages:e}}function z(e,t){h=t;d=e;p=null;if(t===false){s.reverse()}s.sort(e);if(t===false){s.reverse()}u={};j();Dt()}function W(e,t){h=t;p=e;d=null;var n=Object.prototype.toString;Object.prototype.toString=typeof e=="function"?e:function(){return this[e]};if(t===false){s.reverse()}s.sort();Object.prototype.toString=n;if(t===false){s.reverse()}u={};j();Dt()}function X(){if(d){z(d,h)}else if(p){W(p,h)}}function V(e){f=e;if(t.inlineFilters){b=Ct();w=kt()}Dt()}function J(){return x}function K(n){if(!t.groupItemMetadataProvider){t.groupItemMetadataProvider=new Slick.Data.GroupItemMetadataProvider}T=[];N=[];n=n||[];x=n instanceof Array?n:[n];for(var r=0;r<x.length;r++){var i=x[r]=e.extend(true,{},S,x[r]);i.getterIsAFn=typeof i.getter==="function";i.compiledAccumulators=[];var s=i.aggregators.length;while(s--){i.compiledAccumulators[s]=Nt(i.aggregators[s])}N[r]={}}Dt()}function Q(e,t,n){if(e==null){K([]);return}K({getter:e,formatter:t,comparer:n})}function G(e,t){if(!x.length){throw new Error("At least one grouping must be specified before calling setAggregators().")}x[0].aggregators=e;x[0].aggregateCollapsed=t;K(x)}function Y(e){return s[e]}function Z(e){return u[e]}function et(){if(!a){a={};for(var e=0,t=o.length;e<t;e++){a[o[e][i]]=e}}}function tt(e){et();return a[e]}function nt(e){return s[u[e]]}function rt(e){var t=[];et();for(var n=0,r=e.length;n<r;n++){var i=a[e[n]];if(i!=null){t[t.length]=i}}return t}function it(e){var t=[];for(var n=0,r=e.length;n<r;n++){if(e[n]<o.length){t[t.length]=o[e[n]][i]}}return t}function st(e,t){if(u[e]===undefined||e!==t[i]){throw"Invalid or non-matching id"}s[u[e]]=t;if(!l){l={}}l[e]=true;Dt()}function ot(e,t){s.splice(e,0,t);j(e);Dt()}function ut(e){s.push(e);j(s.length-1);Dt()}function at(e){var t=u[e];if(t===undefined){throw"Invalid id"}delete u[e];s.splice(t,1);j(t);Dt()}function ft(){return o.length}function lt(e){var t=o[e];if(t&&t.__group&&t.totals&&!t.totals.initialized){var n=x[t.level];if(!n.displayTotalsRow){wt(t.totals);t.title=n.formatter?n.formatter(t):t.value}}else if(t&&t.__groupTotals&&!t.initialized){wt(t)}return t}function ct(e){var n=o[e];if(n===undefined){return null}if(n.__group){return t.groupItemMetadataProvider.getGroupRowMetadata(n)}if(n.__groupTotals){return t.groupItemMetadataProvider.getTotalsRowMetadata(n)}return null}function ht(e,t){if(e==null){for(var n=0;n<x.length;n++){N[n]={};x[n].collapsed=t}}else{N[e]={};x[e].collapsed=t}Dt()}function pt(e){ht(e,true)}function dt(e){ht(e,false)}function vt(e,t,n){N[e][t]=x[e].collapsed^n;Dt()}function mt(e){var t=Array.prototype.slice.call(arguments);var n=t[0];if(t.length==1&&n.indexOf(C)!=-1){vt(n.split(C).length-1,n,true)}else{vt(t.length-1,t.join(C),true)}}function gt(e){var t=Array.prototype.slice.call(arguments);var n=t[0];if(t.length==1&&n.indexOf(C)!=-1){vt(n.split(C).length-1,n,false)}else{vt(t.length-1,t.join(C),false)}}function yt(){return T}function bt(e,t){var n;var r;var i=[];var s={};var o;var u=t?t.level+1:0;var a=x[u];for(var f=0,l=a.predefinedValues.length;f<l;f++){r=a.predefinedValues[f];n=s[r];if(!n){n=new Slick.Group;n.value=r;n.level=u;n.groupingKey=(t?t.groupingKey+C:"")+r;i[i.length]=n;s[r]=n}}for(var f=0,l=e.length;f<l;f++){o=e[f];r=a.getterIsAFn?a.getter(o):o[a.getter];n=s[r];if(!n){n=new Slick.Group;n.value=r;n.level=u;n.groupingKey=(t?t.groupingKey+C:"")+r;i[i.length]=n;s[r]=n}n.rows[n.count++]=o}if(u<x.length-1){for(var f=0;f<i.length;f++){n=i[f];n.groups=bt(n.rows,n)}}i.sort(x[u].comparer);return i}function wt(e){var t=e.group;var n=x[t.level];var r=t.level==x.length;var i,s=n.aggregators.length;if(!r&&n.aggregateChildGroups){var o=t.groups.length;while(o--){if(!t.groups[o].initialized){wt(t.groups[o])}}}while(s--){i=n.aggregators[s];i.init();if(!r&&n.aggregateChildGroups){n.compiledAccumulators[s].call(i,t.groups)}else{n.compiledAccumulators[s].call(i,t.rows)}i.storeResult(e)}e.initialized=true}function Et(e){var t=x[e.level];var n=new Slick.GroupTotals;n.group=e;e.totals=n;if(!t.lazyTotalsCalculation){wt(n)}}function St(e,t){t=t||0;var n=x[t];var r=n.collapsed;var i=N[t];var s=e.length,o;while(s--){o=e[s];if(o.collapsed&&!n.aggregateCollapsed){continue}if(o.groups){St(o.groups,t+1)}if(n.aggregators.length&&(n.aggregateEmpty||o.rows.length||o.groups&&o.groups.length)){Et(o)}o.collapsed=r^i[o.groupingKey];o.title=n.formatter?n.formatter(o):o.value}}function xt(e,t){t=t||0;var n=x[t];var r=[],i,s=0,o;for(var u=0,a=e.length;u<a;u++){o=e[u];r[s++]=o;if(!o.collapsed){i=o.groups?xt(o.groups,t+1):o.rows;for(var f=0,l=i.length;f<l;f++){r[s++]=i[f]}}if(o.totals&&n.displayTotalsRow&&(!o.collapsed||n.aggregateCollapsed)){r[s++]=o.totals}}return r}function Tt(e){var t=/^function[^(]*\(([^)]*)\)\s*{([\s\S]*)}$/;var n=e.toString().match(t);return{params:n[1].split(","),body:n[2]}}function Nt(e){var t=Tt(e.accumulate);var n=new Function("_items","for (var "+t.params[0]+", _i=0, _il=_items.length; _i<_il; _i++) {"+t.params[0]+" = _items[_i]; "+t.body+"}");n.displayName=n.name="compiledAccumulatorLoop";return n}function Ct(){var e=Tt(f);var t=e.body.replace(/return false\s*([;}]|$)/gi,"{ continue _coreloop; }$1").replace(/return true\s*([;}]|$)/gi,"{ _retval[_idx++] = $item$; continue _coreloop; }$1").replace(/return ([^;}]+?)\s*([;}]|$)/gi,"{ if ($1) { _retval[_idx++] = $item$; }; continue _coreloop; }$2");var n=["var _retval = [], _idx = 0; ","var $item$, $args$ = _args; ","_coreloop: ","for (var _i = 0, _il = _items.length; _i < _il; _i++) { ","$item$ = _items[_i]; ","$filter$; ","} ","return _retval; "].join("");n=n.replace(/\$filter\$/gi,t);n=n.replace(/\$item\$/gi,e.params[0]);n=n.replace(/\$args\$/gi,e.params[1]);var r=new Function("_items,_args",n);r.displayName=r.name="compiledFilter";return r}function kt(){var e=Tt(f);var t=e.body.replace(/return false\s*([;}]|$)/gi,"{ continue _coreloop; }$1").replace(/return true\s*([;}]|$)/gi,"{ _cache[_i] = true;_retval[_idx++] = $item$; continue _coreloop; }$1").replace(/return ([^;}]+?)\s*([;}]|$)/gi,"{ if ((_cache[_i] = $1)) { _retval[_idx++] = $item$; }; continue _coreloop; }$2");var n=["var _retval = [], _idx = 0; ","var $item$, $args$ = _args; ","_coreloop: ","for (var _i = 0, _il = _items.length; _i < _il; _i++) { ","$item$ = _items[_i]; ","if (_cache[_i]) { ","_retval[_idx++] = $item$; ","continue _coreloop; ","} ","$filter$; ","} ","return _retval; "].join("");n=n.replace(/\$filter\$/gi,t);n=n.replace(/\$item\$/gi,e.params[0]);n=n.replace(/\$args\$/gi,e.params[1]);var r=new Function("_items,_args,_cache",n);r.displayName=r.name="compiledFilterWithCaching";return r}function Lt(e,t){var n=[],r=0;for(var i=0,s=e.length;i<s;i++){if(f(e[i],t)){n[r++]=e[i]}}return n}function At(e,t,n){var r=[],i=0,s;for(var o=0,u=e.length;o<u;o++){s=e[o];if(n[o]){r[i++]=s}else if(f(s,t)){r[i++]=s;n[o]=true}}return r}function Ot(e){if(f){var n=t.inlineFilters?b:Lt;var r=t.inlineFilters?w:At;if(v.isFilterNarrowing){y=n(y,g)}else if(v.isFilterExpanding){y=r(e,g,E)}else if(!v.isFilterUnchanged){y=n(e,g)}}else{y=k?e:e.concat()}var i;if(k){if(y.length<L*k){L=Math.floor(y.length/k)}i=y.slice(k*L,k*L+k)}else{i=y}return{totalRows:y.length,rows:i}}function Mt(e,t){var n,r,s,o=[];var u=0,a=t.length;if(v&&v.ignoreDiffsBefore){u=Math.max(0,Math.min(t.length,v.ignoreDiffsBefore))}if(v&&v.ignoreDiffsAfter){a=Math.min(t.length,Math.max(0,v.ignoreDiffsAfter))}for(var f=u,c=e.length;f<a;f++){if(f>=c){o[o.length]=f}else{n=t[f];r=e[f];if(x.length&&(s=n.__nonDataRow||r.__nonDataRow)&&n.__group!==r.__group||n.__group&&!n.equals(r)||s&&(n.__groupTotals||r.__groupTotals)||n[i]!=r[i]||l&&l[n[i]]){o[o.length]=f}}}return o}function _t(e){a=null;if(v.isFilterNarrowing!=m.isFilterNarrowing||v.isFilterExpanding!=m.isFilterExpanding){E=[]}var t=Ot(e);A=t.totalRows;var n=t.rows;T=[];if(x.length){T=bt(n);if(T.length){St(T);n=xt(T)}}var r=Mt(o,n);o=n;return r}function Dt(){if(c){return}var e=o.length;var t=A;var r=_t(s,f);if(k&&A<L*k){L=Math.max(0,Math.ceil(A/k)-1);r=_t(s,f)}l=null;m=v;v={};if(t!=A){_.notify(U(),null,n)}if(e!=o.length){O.notify({previous:e,current:o.length},null,n)}if(r.length>0){M.notify({rows:r},null,n)}}function Pt(t,n,r){function a(e){if(o.join(",")==e.join(",")){return}o=e;u.notify({grid:t,ids:o},new Slick.EventData,i)}function f(){if(o.length>0){s=true;var e=i.mapIdsToRows(o);if(!n){a(i.mapRowsToIds(e))}t.setSelectedRows(e);s=false}}var i=this;var s;var o=i.mapRowsToIds(t.getSelectedRows());var u=new Slick.Event;t.onSelectedRowsChanged.subscribe(function(n,u){if(s){return}var f=i.mapRowsToIds(t.getSelectedRows());if(!r||!t.getOptions().multiSelect){a(f)}else{var l=e.grep(o,function(e){return i.getRowById(e)===undefined});a(l.concat(f))}});this.onRowsChanged.subscribe(f);this.onRowCountChanged.subscribe(f);return u}function Ht(e,t){function s(e){n={};for(var t in e){var r=o[t][i];n[r]=e[t]}}function u(){if(n){r=true;et();var i={};for(var s in n){var o=a[s];if(o!=undefined){i[o]=n[s]}}e.setCellCssStyles(t,i);r=false}}var n;var r;s(e.getCellCssStyles(t));e.onCellCssStylesChanged.subscribe(function(e,n){if(r){return}if(t!=n.key){return}if(n.hash){s(n.hash)}});this.onRowsChanged.subscribe(u);this.onRowCountChanged.subscribe(u)}var n=this;var r={groupItemMetadataProvider:null,inlineFilters:false};var i="id";var s=[];var o=[];var u={};var a=null;var f=null;var l=null;var c=false;var h=true;var p;var d;var v={};var m={};var g;var y=[];var b;var w;var E=[];var S={getter:null,formatter:null,comparer:function(e,t){return e.value-t.value},predefinedValues:[],aggregators:[],aggregateEmpty:false,aggregateCollapsed:false,aggregateChildGroups:false,collapsed:false,displayTotalsRow:true,lazyTotalsCalculation:false};var x=[];var T=[];var N=[];var C=":|:";var k=0;var L=0;var A=0;var O=new Slick.Event;var M=new Slick.Event;var _=new Slick.Event;t=e.extend(true,{},r,t);e.extend(this,{beginUpdate:D,endUpdate:P,setPagingOptions:R,getPagingInfo:U,getItems:I,setItems:q,setFilter:V,sort:z,fastSort:W,reSort:X,setGrouping:K,getGrouping:J,groupBy:Q,setAggregators:G,collapseAllGroups:pt,expandAllGroups:dt,collapseGroup:mt,expandGroup:gt,getGroups:yt,getIdxById:Z,getRowById:tt,getItemById:nt,getItemByIdx:Y,mapRowsToIds:it,mapIdsToRows:rt,setRefreshHints:H,setFilterArgs:B,refresh:Dt,updateItem:st,insertItem:ot,addItem:ut,deleteItem:at,syncGridSelection:Pt,syncGridCellCssStyles:Ht,getLength:ft,getItem:lt,getItemMetadata:ct,onRowCountChanged:O,onRowsChanged:M,onPagingInfoChanged:_})}function n(e){this.field_=e;this.init=function(){this.count_=0;this.nonNullCount_=0;this.sum_=0};this.accumulate=function(e){var t=e[this.field_];this.count_++;if(t!=null&&t!==""&&t!==NaN){this.nonNullCount_++;this.sum_+=parseFloat(t)}};this.storeResult=function(e){if(!e.avg){e.avg={}}if(this.nonNullCount_!=0){e.avg[this.field_]=this.sum_/this.nonNullCount_}}}function r(e){this.field_=e;this.init=function(){this.min_=null};this.accumulate=function(e){var t=e[this.field_];if(t!=null&&t!==""&&t!==NaN){if(this.min_==null||t<this.min_){this.min_=t}}};this.storeResult=function(e){if(!e.min){e.min={}}e.min[this.field_]=this.min_}}function i(e){this.field_=e;this.init=function(){this.max_=null};this.accumulate=function(e){var t=e[this.field_];if(t!=null&&t!==""&&t!==NaN){if(this.max_==null||t>this.max_){this.max_=t}}};this.storeResult=function(e){if(!e.max){e.max={}}e.max[this.field_]=this.max_}}function s(e){this.field_=e;this.init=function(){this.sum_=null};this.accumulate=function(e){var t=e[this.field_];if(t!=null&&t!==""&&t!==NaN){this.sum_+=parseFloat(t)}};this.storeResult=function(e){if(!e.sum){e.sum={}}e.sum[this.field_]=this.sum_}}e.extend(true,window,{Slick:{Data:{DataView:t,Aggregators:{Avg:n,Min:r,Max:i,Sum:s}}}})})(jQuery)
"""


if __name__ == "__main__":

    # This part of the script is not used when the hexagon-profiler front-end,
    # since it calls do_main() directly

    argv = sys.argv[0:]
    argc = len(argv);

    if ((argc <= 4) or (argc >= 8)): die_usage()

    verbose          = 'False'
    showJumpToFuncs  = 'True'
    showLeadingZeros = 'False'
    showPacketBraces = 'False'
    show_0x          = 'False'
    fixChromeMagnify = 'True'
    dockMenu         = 'False'
    underlineJumps   = 'True'
    highlightJumps   = 'False'
    showSelectedType = 'False'
    tools_dir        = ''

    cmd       = argv[0]
    flag      = argv[1]
    fn_json   = argv[2]
    fn_elf    = argv[3]
    fn_html   = argv[4]

    if (argc > 5):
        tools_dir = argv[5]

    for i in range (6, argc):
        if (argv[i] == "--verbose"):
            verbose = 'True'
            continue

        if (argv[i] == "--showLeadingZeros"):
            showLeadingZeros = 'True'
            continue

        if (argv[i] == "--show_0x"):
            show_0x = 'True'
            continue

        if (argv[i] == "--showPacketBraces"):
            showPacketBraces = 'True'
            continue

        if (argv[i] == "--hideJumpToFuncs"):
            showJumpToFuncs = 'False'
            continue

        if (argv[i] == "--noChromeMagnifyFix"):
            fixChromeMagnify = 'False'
            continue

        if (argv[i] == "--dockMenu"):
            dockMenu = 'True'
            continue

        if (argv[i] == "--noUnderlineJumps"):
            underlineJumps = 'False'
            continue

        if (argv[i] == "--highlightJumps"):
            highlightJumps = 'False'
            continue

        if (argv[i] == "--showSelectedType"):
            showSelectedType = 'False'
            continue

    do_main(cmd, flag, fn_json, fn_elf, fn_html, tools_dir, verbose, showLeadingZeros, show_0x, '', showPacketBraces, showJumpToFuncs, fixChromeMagnify, dockMenu, underlineJumps, highlightJumps, showSelectedType)
