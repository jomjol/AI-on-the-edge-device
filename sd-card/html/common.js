 
var basepath = "http://192.168.178.22";
        
function LoadHostname() {
    _basepath = getbasepath(); 


    var xhttp = new XMLHttpRequest();
    xhttp.addEventListener('load', function(event) {
        if (xhttp.status >= 200 && xhttp.status < 300) {
            hostname = xhttp.responseText;
                document.title = hostname + " - AI on the edge";
                document.getElementById("id_title").innerHTML  = "Digitizer - AI on the edge - " + hostname;
        } 
        else {
                console.warn(request.statusText, request.responseText);
        }
    });

//     var xhttp = new XMLHttpRequest();
    try {
            url = _basepath + '/version?type=Hostname';     
            xhttp.open("GET", url, true);
            xhttp.send();

    }
    catch (error)
    {
//               alert("Loading Hostname failed");
    }
}


function LoadVersion() {
    _basepath = getbasepath(); 

    var xhttp = new XMLHttpRequest();
    xhttp.addEventListener('load', function(event) {
        if (xhttp.status >= 200 && xhttp.status < 300) {
            version = xhttp.responseText;
            document.getElementById("Version").innerHTML  = version;
        } 
        else {
                console.warn(request.statusText, request.responseText);
        }
    });

    try {
            url = _basepath + '/version?type=GitBaseBranch';     
            xhttp.open("GET", url, true);
            xhttp.send();

    }
    catch (error)
    {}
}
