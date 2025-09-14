// Note that menu elements cannot have trailing commas after the last item in the
// array otherwise an extra, undefined, element is appended. In IE this manifests
// as the sidebar not being rendered.
var menuElements = [
    [
        "RW Filesystem", [
            ["Overview", "overview.html"],
            ["Home", "index.html"],
            ["Change Log", "changelog.html"],
            ["Email RW Filesystem Support", "mailto:EATechCoreSupport@ea.com"]
        ]
    ],
    [
        "Files", [
            ["Configuring", "file/configuring.html"],
            ["Asynchronous Operations", "file/asyncop.html"],
            ["Batches", "file/batches.html"],
            ["Devices", "file/devices.html"],
            ["Searching", "file/searching.html"]
        ]
    ],
    [
        "Stream", [
            ["Overview", "stream/overview.html"],
            ["Create", "stream/create.html"],
            ["Filters", "stream/filters.html"],
            ["Destroy", "stream/destroy.html"],
            ["Start", "stream/start.html"],
            ["Pointer", "stream/pointer.html"],
            ["Release", "stream/release.html"],
            ["Coordinate", "stream/coordinate.html"],
            ["Alignment", "stream/alignment.html"],
            ["Performance", "stream/performance.html"]
        ]
    ],
    [
        "User Guides", [
            ["Incremental Installer", "userguide/incrementalinstaller.html"],
            ["Memory Mapped Device Driver", "userguide/memmapdevicedriver.html"]
        ]
    ]
];

function buildCategory(categoryData, linkPrefix) {
    var categoryName = categoryData[0];
    var categoryContents = categoryData[1];

    var categoryText = "<h3>" + categoryName + "</h3>";
    categoryText += "<ul>";

    for (var i = 0; i < categoryContents.length; ++i) {
        var linkText = categoryContents[i][0];
        var linkURL = categoryContents[i][1];
        categoryText += "<li>";

        if (linkURL.search("mailto") == -1) {
            linkURL = linkPrefix + linkURL;
        }

        categoryText += linkText.link(linkURL);
        categoryText += "</li>";
    }

    categoryText += "</ul>";
    return categoryText;
}

function buildMenu(linkPrefix) {
    var menu = "";

    for (var i = 0; i < menuElements.length; ++i) {
        menu += buildCategory(menuElements[i], linkPrefix);
    }

    return menu;
}

function loadMenu(linkPrefix) {
    var menuBar = document.getElementById("menubar");
    menuBar.innerHTML = buildMenu(linkPrefix);

    document.getElementById("header").innerHTML = "<img src=\"" + linkPrefix + "img/logo.png\"/>";
    document.getElementById("footer").innerHTML = "<center><h4>Copyright (c) Electronic Arts.</h4></center>";
}
