
function gethost_Version(){
    return "1.0.0 - 20200910";
}


function getbasepath(){
    var host = window.location.protocol + "//" + window.location.hostname;
    if (window.location.port != "") {
        host = host + ":" + window.location.port;
    }

    return host;
}

function UpdatePage(_dosession = true){
    var zw = location.href;
    zw = zw.substr(0, zw.indexOf("?"));
    if (_dosession) {
        window.location = zw + '?session=' + Math.floor((Math.random() * 1000000) + 1); 
    }
    else {
        window.location = zw; 
    }
}
