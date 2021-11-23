
function gethost_Version(){
    return "1.0.0 - 20200910";
}


function getbasepath(){
    var host = window.location.hostname;
    if (((host == "127.0.0.1") || (host == "localhost") || (host == "")) 
       && ((window.location.port == "80") || (window.location.port == "")))
    
    {
//        host = "http://192.168.2.219";          // jomjol interner test
//        host = "http://192.168.178.46";          // jomjol interner test
        host = "http://192.168.178.79";          // jomjol interner Real
//        host = "http://192.168.43.191";
//        host = ".";                           // jomjol interner localhost   

    }
    else
    {
        host = "http://" + host;
    }

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
