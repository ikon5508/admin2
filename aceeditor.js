var editor = ace.edit("editor");
editor.setTheme("ace/theme/monokai");


var tools_togglestate = 0;
var state_togglestate = 0;
var make_togglestate = 0;
var copiedtext;
var stateobj = document.getElementById("state");
var makeobj = document.getElementById("make");
var toolsobj = document.getElementById("tools");

    var editor = ace.edit("editor");
    editor.setTheme("ace/theme/terminal");
    
    if (ext == "c" || ext == "h" || ext == "cpp" || ext == "hpp")
    editor.session.setMode("ace/mode/c_cpp");
    
    if (ext == "htm")
    editor.session.setMode("ace/mode/html");
    
    if (ext == "rs")
    editor.session.setMode("ace/mode/rust");
    
    document.getElementById('editor').style.fontSize=("18px");
    editor.session.setUseWrapMode(true);

//window.alert (ace.version);

fetch (rpath)
.then(res => res.text()) 
.then(res => editor.setValue(res))
.catch(err => window.alert(err));

//let a = sessionStorage.getItem ("item1");
//sessionStorage.setItem ("item1", i);
//localStorage.removeItem ("item1");


function test_func () {
alert (getLine());

    stateobj.style.display = "none";
    state_togglestate = 0;
//alert (editor.selection.getCursor());
	//alert (editor.selection.getRange());
}

function clear_state () {
localStorage.removeItem ("content");
localStorage.removeItem ("line");
stateobj.style.display = "none";
state_togglestate = 0;
}
function save_state () {
localStorage.setItem ("content", editor.getValue());
localStorage.setItem ("line", getLine());
stateobj.style.display = "none";
state_togglestate = 0;
}
function restore_state () {
editor.setValue(localStorage.getItem ("content"));
editor.gotoLine (localStorage.getItem ("line"));
stateobj.style.display = "none";
state_togglestate = 0;
}

function save_fetch ()
{
window.alert ("test function");

let a = editor.getValue(); // or session.getValue
//window.alert (a);

let url = window.location + "?fetch";

fetch (url, {method: "POST", body: JSON.stringify (a)})
.then(res => res.text()) 
.then(res => window.alert(res))

.catch(err => window.alert(err));
toggle_tools ();
} // function save_fetch


function save_post ()
{
window.alert ("test function");

let a = editor.getValue(); // or session.getValue
 
form = document.createElement('form');
form.setAttribute('method', 'POST');
form.setAttribute('enctype', 'multipart/form-data');
form.setAttribute('action', window.location);

myvar = document.createElement('input');
myvar.setAttribute('name', 'editor');
myvar.setAttribute('type', 'hidden');
myvar.setAttribute('value', a);

form.appendChild(myvar);
document.body.appendChild(form);
form.submit();   
toggle_tools ();
//window.alert (a);
} // function save_post

function getLine () {
let range_text = new String (editor.selection.getRange());
let d1 = range_text.indexOf ('[');
let d2 = range_text.indexOf ('/');
let line = range_text.slice (d1 + 1, d2);
return line;
}

/*
function bookmark (evt)
{
//window.alert (evt.target.value);
//let path = "ace/mode/" + evt.target.value;
  // editor.session.setMode(path);
editor.gotoLine (evt.target.value);
off ();

}
function theme (evt) {
//window.alert (evt.target.value);

let path = "ace/theme/" + evt.target.value;
editor.setTheme(path);

}
*/


function jump () {
    let ln = prompt ("what line?");
    editor.gotoLine (ln);
    toolsobj.style.display = "none";
    tools_togglestate = 0;
}

function cut () {
copiedtext = editor.getCopyText ();
editor.insert ("");
    toolsobj.style.display = "none";
    tools_togglestate = 0;
}
function copy () {
    copiedtext = editor.getCopyText ();
    toolsobj.style.display = "none";
    tools_togglestate = 0;
}

function paste () {
    editor.insert (copiedtext);
    toolsobj.style.display = "none";
    tools_togglestate = 0;
}
function fnt_sz (evt) {
   let ssz = evt.target.value + "px"; 
    document.getElementById('editor').style.fontSize=(ssz);
    toolsobj.style.display = "none";
    tools_togglestate = 0;
}

function ro () {
var ab = document.getElementById ("cb");
    
if (ab.checked == true)
editor.setReadOnly (true);
        
if (ab.checked == false)
editor.setReadOnly (false);
  
    toolsobj.style.display = "none";
    tools_togglestate = 0;
}


function toggle_state () {

if (state_togglestate == 0){
    stateobj.style.display = "block";
    state_togglestate = 1;
}else if (state_togglestate == 1) {
    stateobj.style.display = "none";
    state_togglestate = 0;
}

}
function toggle_make () {

if (make_togglestate == 0){
    makeobj.style.display = "block";
    make_togglestate = 1;
}else if (make_togglestate == 1) {
    makeobj.style.display = "none";
    make_togglestate = 0;
}
tools_togglestate = 0;
toolsobj.style.display = "none";
}

function toggle_tools () {
if (tools_togglestate == 0){
    toolsobj.style.display = "block";
    tools_togglestate = 1;
}else if (tools_togglestate == 1) {
    toolsobj.style.display = "none";
    tools_togglestate = 0;
}
}
