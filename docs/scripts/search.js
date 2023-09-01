<!-- hide script from old browsers

function getParameterByName(name) {
    name = name.replace(/[\[]/, "\\[").replace(/[\]]/, "\\]");
    var regex = new RegExp("[\\?&]" + name + "=([^&#]*)"),
        results = regex.exec(location.search);
    return results == null ? "" : decodeURIComponent(results[1].replace(/\+/g, " "));
}

function searchDocs(text){
    var html = ""

    if(text){
        results = idx.search(text)
        for (var i = 0; i < results.length; i++) {
            var id = parseInt(results[i].ref, 10)
            html = html + "<a target=&quot;_blank&quot; href=" + urls[id] + ">"+ titles[id] + "</a><br/>"
        }
    }
    else{
        html = html + "No results found."
    }

    var container = document.getElementById('searchresults');
    container.innerHTML = html;
}

$( "#searchForm" ).submit(function( event ) {
    searchDocs($( "input:first" ).val())
    event.preventDefault();
});

$('#searchButton').click(function(){
    searchDocs($( "input:first" ).val())
    event.preventDefault();
});

// Handle page load with parameter
$( document ).ready(function() {
    var query = getParameterByName("query")
    if (query){
        searchDocs(query);        
        event.preventDefault();
    }
});
// end hiding script from old browsers -->
