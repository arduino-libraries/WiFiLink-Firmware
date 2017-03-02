function fetchText(a, c) {
    var b = $("#console");
    if (b.textEnd == undefined) {
        b.textEnd = 0;
        b.innerHTML = ""
    }
    window.setTimeout(function() {
        //   // ajaxJson("GET", console_url + "?start=" + b.textEnd, function(e) {
        //    ajaxJson("GET", console_url, function(e) {
        //        var d = updateText(e);
        //        if (c) {
        //            fetchText(d, c)
        //        }
        //    }, function() {
        //        retryLoad(c)
        //    })
        //}, a)

        window.setInterval(function() {
            var xhr = new XMLHttpRequest();
            xhr.open("GET", console_url, true);
            xhr.onload = function (e) {
                if (xhr.readyState === 4) {
                    if(xhr.responseText.length > 0) {
                        if (xhr.status === 200) {
                            console.log(xhr.responseText);
                            updateText(xhr.responseText);
                        } else {
                            console.error(xhr.statusText);
                        }
                    }
                }
            };
            xhr.onerror = function (e) {
                console.error(xhr.statusText);
            };

            xhr.send();
        }, 100);

    }, a );

}

function updateText(e) {
    var c = $("#console");
    var a;
    if ($("#input-scroll") == null) {
        a = true
    } else {
        a = $("#input-scroll").checked
    }
    var b = 3000;

    c.innerHTML = c.innerHTML.concat(e);
    return b
}

function updateTextOLD(e) {
    var c = $("#console");
    var a;
    if ($("#input-scroll") == null) {
        a = true
    } else {
        a = $("#input-scroll").checked
    }
    var b = 3000;
    if (e != null && e.len > 0) {
        var d = c.scrollHeight - c.clientHeight <= c.scrollTop + 1;
        if (e.start > c.textEnd) {
            c.innerHTML = c.innerHTML.concat("\r\n<missing lines\r\n")
        }
        c.innerHTML = c.innerHTML.concat(e.text);
        c.textEnd = e.start + e.len;
        b = 500;
        if (d && a) {
            c.scrollTop = c.scrollHeight - c.clientHeight
        }
    }
    return b
}

function retryLoad(a) {
    fetchText(1000, a)
}

function consoleSendInit() {
    var e = $("#send-history");
    var h = $("#input-text");
    var d = $("#input-add-cr");
    var b = $("#input-add-lf");

    function g(k) {
        for (var j = 0; j < e.children.length; j++) {
            if (k == e.children[j].value) {
                return j
            }
        }
        return null
    }

    function a(i) {
        e.value = e.children[i].value;
        h.value = e.children[i].value
    }

    function f(j) {
        var i = g(e.value) + j;
        if (i < 0) {
            i = e.children.length - 1
        }
        if (i >= e.children.length) {
            i = 0
        }
        a(i)
    }
    e.addEventListener("change", function(i) {
        h.value = e.value
    });

    function c(k) {
        var i = g(k);
        if (i !== null) {
            a(i);
            return false
        }
        var j = m("<option>" + (k.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;").replace(/"/g, "&quot;")) + "</option>");
        j.value = k;
        e.appendChild(j);
        e.value = k;
        for (; e.children.length > 15;) {
            e.removeChild(e.children[0])
        }
        return true
    }
    h.addEventListener("keydown", function(i) {
        switch (i.keyCode) {
            case 38:
                i.preventDefault();
                f(-1);
                break;
            case 40:
                i.preventDefault();
                f(+1);
                break;
            case 27:
                i.preventDefault();
                h.value = "";
                e.value = "";
                break;
            case 13:
                i.preventDefault();
                var j = h.value;
                if (d.checked) {
                    j += "\r"
                }
                if (b.checked) {
                    j += "\n"
                }
                c(h.value);
                h.value = "";
                ajaxSpin("POST", "/console/send?text=" + encodeURIComponent(j), function(k) {
                //ajaxSpin("POST", "/console/send/" + encodeURIComponent(j), function(k) {
                    showNotification("Text sent")
                }, function(l, k) {
                    showWarning("Error sending text")
                });
                break
        }
    })
}

function clearConsole() {
    var a = $("#console");
    a.innerHTML = ""
}

function showDbgMode(c) {
    var b = $(".dbg-btn");
    for (var a = 0; a < b.length; a++) {
        if (b[a].id === "dbg-" + c) {
            addClass(b[a], "button-selected")
        } else {
            removeClass(b[a], "button-selected")
        }
    }
};
