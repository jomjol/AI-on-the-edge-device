 
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
            url = _basepath + '/info?type=Hostname';     
            xhttp.open("GET", url, true);
            xhttp.send();

    }
    catch (error)
    {
//               alert("Loading Hostname failed");
    }
}


var fwVersion = "";
var webUiVersion = "";

function LoadFwVersion() {
    _basepath = getbasepath(); 

    var xhttp = new XMLHttpRequest();
    xhttp.addEventListener('load', function(event) {
        if (xhttp.status >= 200 && xhttp.status < 300) {
            fwVersion = xhttp.responseText;
            document.getElementById("Version").innerHTML  = fwVersion;
            console.log(fwVersion);
            compareVersions();
        } 
        else {
            console.warn(request.statusText, request.responseText);
            fwVersion = "NaN";
        }
    });

    try {
        url = _basepath + '/info?type=GitBaseBranch';     
        xhttp.open("GET", url, true);
        xhttp.send();
    }
    catch (error) {
        fwVersion = "NaN";
    }
}

function LoadWebUiVersion() {
    _basepath = getbasepath(); 

    var xhttp = new XMLHttpRequest();
    xhttp.addEventListener('load', function(event) {
        if (xhttp.status >= 200 && xhttp.status < 300) {
            webUiVersion = xhttp.responseText;
            console.log("Web UI Version: " + webUiVersion);
            compareVersions();
        } 
        else {
            console.warn(request.statusText, request.responseText);
            webUiVersion = "NaN";
        }
    });

    try {
        url = _basepath + '/info?type=HTMLVersion';     
        xhttp.open("GET", url, true);
        xhttp.send();
    }
    catch (error) {
        webUiVersion = "NaN";
    }
}


function compareVersions() {
    if (fwVersion == "" || webUiVersion == "") {
        return;
    }

    arr = fwVersion.split(" ");
    fWGitHash = arr[arr.length - 1].substring(0, 7);
    arr = webUiVersion.split(" ");
    webUiHash = arr[arr.length - 1].substring(0, 7);
    console.log("FW Hash: " + fWGitHash + ", Web UI Hash: " + webUiHash);
    
    if (fWGitHash != webUiHash) {
        alert("The Version of the Web Interface does not match the Firmware Version! It is suggested to keep them on the same version!");
    }
}
