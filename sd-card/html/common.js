 
/* The UI can also be run locally, but you have to set the IP of your devide accordingly.
 * And you also might have to disable CORS in your webbrowser! */
var domainname_for_testing = "192.168.178.23";



function gethost_Version(){
    return "1.0.0 - 20200910";
}


/* Returns the domainname with prepended protocol.
Eg. http://watermeter.fritz.box or http://192.168.1.5 */
function getDomainname(){
    var host = window.location.hostname;
    if (((host == "127.0.0.1") || (host == "localhost") || (host == "")) 
//       && ((window.location.port == "80") || (window.location.port == ""))
       )
    
    {
        console.log("Using pre-defined domainname for testing: " + domainname_for_testing);
        domainname = "http://" + domainname_for_testing
    }
    else
    {
        domainname = window.location.protocol + "//" + host;
        if (window.location.port != "") {
            domainname = domainname + ":" + window.location.port;
        }
    }

    return domainname;
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

        
function LoadHostname() {
    _domainname = getDomainname(); 


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
            url = _domainname + '/info?type=Hostname';     
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
    _domainname = getDomainname(); 

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
        url = _domainname + '/info?type=FirmwareVersion';     
        xhttp.open("GET", url, true);
        xhttp.send();
    }
    catch (error) {
        fwVersion = "NaN";
    }
}

function LoadWebUiVersion() {
    _domainname = getDomainname(); 

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
        url = _domainname + '/info?type=HTMLVersion';     
        console.log("url");
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
        firework.launch("The Version of the Web Interface (" + webUiHash + 
            ") does not match the Firmware Version (" + 
            fWGitHash + ")! It is suggested to keep them on the same version!", 'warning', 30000);
    }
}
