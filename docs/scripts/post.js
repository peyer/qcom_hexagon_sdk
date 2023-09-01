<!-- hide script from old browsers
function postFunction(isaddon){
    if (location.pathname.substring(1).split("/").slice(-1) == 'search.html'){
        return
    }

    if (isaddon) {
        searchStr = '../search.html?query=' + document.getElementById('searchquery').value;
    }
    else {
        searchStr = 'search.html?query=' + document.getElementById('searchquery').value;
    }
    window.open(searchStr,'_blank');
}

function postFunctionKeydown(event, isaddon) {
    var holder;

    //IE uses this
    if(window.event) {
        holder = window.event.keyCode;
    }
    //FF uses this
    else {
        holder = event.which;
    }

    if(holder === 13) {
     	postFunction(isaddon)
    }
}

// end hiding script from old browsers -->
