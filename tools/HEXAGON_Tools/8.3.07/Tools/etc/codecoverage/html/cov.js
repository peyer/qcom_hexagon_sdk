$(document).ready(function() {
    setTimeout(function() {
        var b = $(document.body);
        if (b.length > 0) {
            b[0].focus();
        }
    }, 250);
    function toggleAllSrcLines(show) {
        $('.expanded_src').each(function() {
            // hide while toggling for performance
            $(this).css('visibility', 'hidden');
            $(this).toggle(show);
            $(this).css('visibility', 'visible');
        });
    }

    var currentSrcLine;
    $('.source_line').each(function() {
        if ($('.packet_table',this).length) {
            $(this).css('cursor','pointer');
            $(this).dblclick(function(e) {
                if (e.ctrlKey || e.metaKey) {
                    toggleAllSrcLines(($('.expanded_src',this).css('display') == 'none'));
                    // scroll to where the user clicked so the same source code is visible
                    $('body,html,document').scrollTop($(this).offset().top - e.clientY);
                } else {
                    $('.expanded_src',this).slideToggle('slow');
                }
            });
            $(this).click(function(e) {
                // clear current selection
                if (typeof currentSrcLine != 'undefined' && currentSrcLine.length != 0) {
                    if (currentSrcLine != this)
                        $(currentSrcLine).toggleClass('src_selected');
                }
                if (currentSrcLine != this) {
                    currentSrcLine = this;
                    $(currentSrcLine).toggleClass('src_selected');
                }
            });
        }
    });

    $(document).keydown(function(e) {
        if (!e.ctrlKey && !e.metaKey){
            return;
        }
        switch(e.which) {
            case 38: //up
                if (typeof currentSrcLine == 'undefined' || currentSrcLine.length == 0) {
                    currentSrcLine = $('.source_line:not(.color_class_blue)').last();
                } else {
                    $(currentSrcLine).toggleClass('src_selected');
                    currentSrcLine = $(currentSrcLine).prevAll('.source_line:not(.color_class_blue)').first();
                }
                break;
            case 40: //down
                if (typeof currentSrcLine == 'undefined' || currentSrcLine.length == 0) {
                    currentSrcLine = $('.source_line:not(.color_class_blue)').first();
                } else {
                    $(currentSrcLine).toggleClass('src_selected');
                    currentSrcLine = $(currentSrcLine).nextAll('.source_line:not(.color_class_blue)').first();
                }
                break;
            default: return;
        }
        if (currentSrcLine.length > 0) {
            // stop the key up/down from being processed, otherwise,
            // key scrolling and scrollTop will combine to incorrect position
            e.preventDefault();
            $('html,body,document').scrollTop($(currentSrcLine[0]).offset().top - 10);
            $(currentSrcLine).toggleClass('src_selected');
        }
    });

    function togglePathName(obj){
        if (typeof obj.orig_text == 'undefined' || obj.orig_text == '') {
            var cur_text = $(obj).text();
            var last_sep = cur_text.lastIndexOf("/");
            if (last_sep > 0) {
                obj.orig_text = cur_text;
                $(obj).text(cur_text.substr(last_sep+1));
            }
        } else {
            $(obj).text(obj.orig_text);
            obj.orig_text = '';
        }
    }

    $('.src_file_path').each(function() {
        togglePathName(this);
    });

    $('.src_file_path').click(function(e) {
        togglePathName(this);
    });

    function toggleAllTocPathNames() {
        $('.toc_src_file_path').each(function() {
            togglePathName(this);
        });
    }

    toggleAllTocPathNames();

    $('.toc_src_file_weight').click(function(e) {
        if (e.ctrlKey || e.metaKey) {
            toggleAllTocPathNames();
        } else {
            // <tr>
            // <td><div class='toc_src_file_weight'>10%</div></td>
            // <td><div class='toc_src_file_path'>/a/file/path.cpp</div></td>
            // </tr>
            var parent = $(this).parent();
            var path = $('.toc_src_file_path', parent);
            togglePathName(path[0]);
        }
    });

    jQuery.extend(jQuery.fn.dataTableExt.oSort, {
        "percent-numeric-pre" : function(x) {
            var val = x.replace(/[%()]/,"");
            return parseFloat(val);
        },
        "percent-numeric-asc" : function(x,y) {
            return ((x < y) ? -1 : ((x > y) ? 1 : 0));
        },
        "percent-numeric-desc" : function(x,y) {
            return ((x < y) ? 1 : ((x > y) ? -1 : 0));
        }
    });
    // make the table searchable and sortable
    $('#toc_table').dataTable({
        "iDisplayLength":50,
        "order": [[0,'asc'],[1,'asc']],
        "columnDefs": [
            {"searchable":false,"visible":false,"targets":0},
            {"searchable":false,"visible":false,"targets":1},
            {"searchable":false,"type":'percent-numeric',"targets":2},
            {"orderData":[ 0, 1 ],"type":'string',"targets":3}
        ],
        "search": {
            "regex": true,
            "smart": false
        },
        "oLanguage": {
            "sLengthMenu": 'Show <select>' +
                '<option value="10">10</option>' +
                '<option value="25">25</option>' +
                '<option value="50">50</option>' +
                '<option value="100">100</option>' +
                '<option value="-1">All</option>' +
                '</select> records'
        }
    });
});
