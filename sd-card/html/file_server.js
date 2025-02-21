function setpath() {
  var fileserverpraefix = "/fileserver";
  var anz_zeichen_fileserver = fileserverpraefix.length;
  var default_path = window.location.pathname.substring(anz_zeichen_fileserver) + document.getElementById("newfile").files[0].name;
  document.getElementById("filepath").value = default_path;
}

function dirup() {
  var str = window.location.href;
  str = str.substring(0, str.length-1);
  var zw = str.indexOf("/");
  var found = zw;
  while (zw >= 0) {
    zw = str.indexOf("/", found+1);  
    if (zw >= 0) { 
      found = zw; 
    }
  }
  var res = str.substring(0, found+1);
  window.location.href = res;	
}

function upload() {
  var filePath = document.getElementById("filepath").value;
  var upload_path = "/upload/" + filePath;
  var fileInput = document.getElementById("newfile").files;

  // Max size of an individual file. Make sure this value is same as that set in file_server.c
  var MAX_FILE_SIZE = 8000*1024;
  var MAX_FILE_SIZE_STR = "8000KB";

  if (fileInput.length == 0) {
    firework.launch('No file selected!', 'danger', 30000);
  } else if (filePath.length == 0) {
    firework.launch('File path on server is not set!', 'danger', 30000);
  } else if (filePath.length > 100) {
    firework.launch('Filename is to long! Max 100 characters.', 'danger', 30000);
  } else if (filePath.indexOf(' ') >= 0) {
    firework.launch('File path on server cannot have spaces!', 'danger', 30000);
  } else if (filePath[filePath.length-1] == '/') {
    firework.launch('File name not specified after path!', 'danger', 30000);
  } else if (fileInput[0].size > MAX_FILE_SIZE) {
    firework.launch("File size must be less than " + MAX_FILE_SIZE_STR + "!", 'danger', 30000);
  } else {
    document.getElementById("newfile").disabled = true;
    document.getElementById("filepath").disabled = true;
    document.getElementById("upload").disabled = true;

    var file = fileInput[0];
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (xhttp.readyState == 4) {
        if (xhttp.status == 200) {
          document.open();
          document.write(xhttp.responseText);
          document.close();
          firework.launch('File upload completed', 'success', 5000);
        } else if (xhttp.status == 0) {
          firework.launch('Server closed the connection abruptly!', 'danger', 30000);
          UpdatePage(false);
        } else {
          firework.launch('An error occured: ' + xhttp.responseText, 'danger', 30000);
          UpdatePage(false);
        }
      }
    };
    xhttp.open("POST", upload_path, true);
    xhttp.send(file);
  }
}

function checkAtRootLevel(res) {
  if (getPath() == "/fileserver/") { 
    // Already at root level
    document.getElementById("dirup").disabled = true;
    console.log("Already on sd-card root level!");
    return true;
  }

  document.getElementById("dirup").disabled = false;
  return false;
}

function getPath() {
  return window.location.pathname.replace(/\/+$/, '') + "/"
}

function initFileServer() {
  checkAtRootLevel();
  console.log("Current path: " + getPath().replace("/fileserver", ""));
  document.getElementById("currentpath").innerHTML = "Current path: <b>" + getPath().replace("/fileserver", "") + "</b>";
  document.cookie = "page=" + getPath() + "; path=/";
}