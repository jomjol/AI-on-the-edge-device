
function getbasepath(){
    var host = window.location.hostname;
    if (host == "127.0.0.1")
    {
        host = "http://192.168.178.26";          // jomjol interner test
//        host = "http://192.168.178.22";          // jomjol interner Real
//        host = ".";                           // jomjol interner localhost   
    }
    else
    {
        host = "http://" + host;
    }
    return host;
}
