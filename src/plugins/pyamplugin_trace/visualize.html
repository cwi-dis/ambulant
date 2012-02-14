<!DOCTYPE html>
<html>
  <head>
    <title>Bar Chart</title>
    <script type="text/javascript" src="d3.js"></script>
    <style type="text/css">

body {
  font: 10px sans-serif;
}

g.bandwidth path {
	fill: orange;
}

g.bar line {
	stroke: #666666;
}

g.bar rect.selected {
	stroke: black;
}

g.playable line {
	stroke: #ffffff;
}

g.bar rect.run {
  fill: #1f77b4;
}

g.bar rect.fill {
  fill: #aec7e8;
}

.legend-rest {
  background-color: #1f77b4;
}

g.medianode rect.run {
  fill: #17becf;
}

g.medianode rect.fill {
  fill: #9edae5;
}

.legend-medianode {
  background-color: #17becf;
}

g.prefetch rect.run {
  fill: #9467bd;
}

g.prefetch rect.fill {
  fill: #c5b0d5;
}

.legend-prefetch {
  background-color: #9467bd;
}

g.playable rect.run {
  fill: #2ca02c;
}

g.playable rect.fill {
  fill: #98df8a;
}

.legend-playable {
  background-color: #2ca02c;
}

g.playable rect.stall {
  fill: #d62728;
}

.bar text.value {
  fill: black;
}

.axis {
  shape-rendering: crispEdges;
}

.axis path {
  fill: none;
}

.x.axis line {
  stroke: #fff;
  stroke-opacity: .8;
}

.y.axis path {
  stroke: black;
}

    </style>
  </head>
  <body>
    <form name="selections">
    	<span class="legend-rest"><input type="checkbox" name="show" value="*" checked="checked" onClick="regenShowAndGraph();">Structure</span>
    	<span class="legend-medianode"><input type="checkbox" name="show" value="medianode" checked="checked" onClick="regenShowAndGraph();">Media</span>
    	<span class="legend-prefetch"><input type="checkbox" name="show" value="prefetch" checked="checked" onClick="regenShowAndGraph();">Prefetch</span>
    	<span class="legend-playable"><input type="checkbox" name="show" value="playable" checked="checked" onClick="regenShowAndGraph();">Renderers</span>
    	<span class="legend-bandwidth"><input type="checkbox" name="show" value="bandwidth" checked="checked" onClick="regenShowAndGraph();">Bandwidth</span>
    	<br><br>
    	<input type="button" value="zoomin time" onClick="zoom(1.414, 1.0);">
    	<input type="button" value="zoomout time" onClick="zoom(0.707, 1.0);">
    	<input type="button" value="larger nodes" onClick="zoom(1.0, 1.414);">
    	<input type="button" value="smaller nodes" onClick="zoom(1.0, 0.707);">
    	<input type="checkbox" name="autoreload" value="*" checked="checked" onClick="autoReloadChanged();">Auto-reload
    	<br><br>
    	Load data: <input name="inputdata" type="text"><input type="button" value="Open Data" onClick="loadData()">
    	<br>
    	Save data: <input type="button" value="Save to New Window" onClick="saveData()">
    </form>
    <div id="graphlocation"></div>
    <script type="text/javascript">

var formatTime = d3.format(".3f");

var dataFile="data.json";
var lastReadData = [];

var m = [30, 10, 10, 330],
    w = 960 - m[1] - m[3],
    h = 930 - m[0] - m[2];

var h_scale = 1.0;
var v_scale = 1.0;

var format = d3.format(",.1f");

var x = d3.scale.linear().range([0, w*h_scale]),
    y = d3.scale.ordinal().rangeRoundBands([0, h*v_scale], .1);

var xAxis = d3.svg.axis().scale(x).orient("top").tickSize(-h),
    yAxis = d3.svg.axis().scale(y).orient("left").tickSize(0);

var svg = d3.selectAll("div#graphlocation").append("svg")
	.attr("width", w*h_scale + m[1] + m[3])
	.attr("height", h*v_scale + m[0] + m[2])
	.append("g")
	.attr("transform", "translate(" + m[3] + "," + m[0] + ")");

var showTypes = new Array();
showTypes["medianode"] = true;
showTypes["prefetch"] = true;
showTypes["playable"] = true;
showTypes["bandwidth"] = true;
showTypes["*"] = true;

svg.append("g")
	.attr("class", "x axis");

svg.append("g")
	.attr("class", "y axis");

var selectFunc = function() {
	// Deselect old selection
	svg.selectAll(".selected")
		.classed("selected", 0);
	// Select the currrent run rectangle
	d3.select(this).selectAll("rect.run")
		.classed("selected", 1);
};

genGraph = function(data) {
	// Keep the data (for saving)
	lastReadData = data;
	
	// Select only the ones we want
	var newData = [];
	var newBandwidthData = [];
	for( var dataItemIndex in data) {
		var dataItem = data[dataItemIndex];
		var objType = dataItem.objtype;
		if (objType in showTypes) {
			if (showTypes[objType])
				newData.push(dataItem);
		} else {
			if (showTypes["*"])
				newData.push(dataItem);
		}
	}
	// Get the global bandwidth data, if needed
	if ("bandwidth" in showTypes && showTypes["bandwidth"]) {
		if ("bandwidth" in data[0].runs[0]) {
			for (var k in data[0].runs[0].bandwidth) {
				var v = data[0].runs[0].bandwidth[k];
				var newbw = new Object();
				newbw.objid = "bandwidth " + k;
				newbw.objtype = "bandwidth";
				newbw.runs = [];
				newbw.stripdata = [[0, 0]];
				var lasttime = 0;
				v.forEach(function(d) {
					newbw.stripdata.push([lasttime, d[1]]);
					newbw.stripdata.push(d);
					lasttime = d[0];
					});
				newbw.stripdata.push([lasttime, 0]);
				newbw.stripdata.push([0, 0]);
				newBandwidthData.unshift(newbw);
			}
		}
	}
	data = [];	
	data = data.concat(newBandwidthData, newData);
	
	// Parse numbers, and sort by value.
	//data.forEach(function(d) { d.runs[0].start = +d.runs[0].start; d.runs[0].stop = +d.runs[0].stop; });
	// data.sort(function(a, b) { return b.start - a.start; });
	
	// Set the scale domain and ranges
	x.domain([0, d3.max(data, function(d) { return d.runs.length ? d.runs[0].stop : 0; })]);
	y.domain(data.map(function(d) { return d.objid; }));
	var new_h = data.length * 3 * 10; // Number(getComputedStyle('graphlocation', '').fontSize.match(/(\d+)px/)[1]);
	// if (new_h < h) new_h = h;
	x.range([0, w*h_scale]);
	y.rangeRoundBands([0, new_h*v_scale], .1);
	
	// Get the graph, and set height and width
	d3.selectAll("div#graphlocation svg")
		.transition().duration(500)
		.attr("width", w*h_scale + m[1] + m[3])
		.attr("height", new_h*v_scale + m[0] + m[2]);
	
	// Set up the bandwidth indicators
	var bwgroup = svg.selectAll("g.bandwidth")
		.data(newBandwidthData, function(d) { return d.objid; });
		
	bwgroup.exit().remove();
	
	bwgroup.enter().append("g")
		.attr("class", "bandwidth")
		.attr("transform", function(d) { console.log("bwdata "+d); return "translate(0," + y(d.objid) + ")"; })
		.append("path")
		.attr("d", function(d) {
			var maxy = d3.max(d.stripdata, function(d) { return d[1]; });
			console.log("max stripdata " + maxy);
			var line = d3.svg.line()
				.x(function(d) { return x(d[0]); })
				.y(function(d) { return y.rangeBand() * (0.5-(d[1]/maxy)); });
			return line(d.stripdata);
		});
		
	bwgroup.selectAll("path")
		.map(function(d) { return this.parentNode.__data__; })
		.attr("d", function(d) {
			var maxy = d3.max(d.stripdata, function(d) { return d[1]; });
			console.log("max stripdata " + maxy);
			var line = d3.svg.line()
				.x(function(d) { console.log("line.x point is" + d); return x(d[0]); })
				.y(function(d) { return y.rangeBand() * (d[1]/maxy); });
			return line(d.stripdata);
		});
		
	// Now set up the bars (rows). Do this separately for existing/new/deleted data.
	var bars = svg.selectAll("g.bar")
		.data(newData, function(d) { return d.objid; })
	
	bars.transition().duration(500)
		.attr("transform", function(d) { return "translate(0," + y(d.objid) + ")"; });

	bars.enter().append("g")
		.attr("class", "bar")
		.classed("medianode", function(d) { return d.objtype == "medianode"; })
		.classed("playable", function(d) { return d.objtype == "playable"; })
		.classed("prefetch", function(d) { return d.objtype == "prefetch"; })
		.attr("transform", function(d) { return "translate(0," + y(d.objid) + ")"; })
		
		.append("line")
		.attr("x1", -10)
		.attr("x2", 10)
		.attr("y1", 0)
		.attr("y2", 0);
		
	bars.exit().remove();

	// Next, set up the grouped data (runs) within a horizontal bar. Again existing/new/removed.
	var rungroup = bars.selectAll("g.rungroup")
		.data(function(d) {return d.runs; });
	
	rungroup.exit().remove();

	// Now modify the rects in the existing rungroups
	var tooltipfunc = function(d) {
		var rv = "node: \t" + d.descr + "\n\n" +
			"begin:\t" + formatTime(d.start) + "\n";
		if (d.fill) rv += "fill:\t\t" + formatTime(d.fill) + "\n";
		rv += "stop: \t" + formatTime(d.stop) + "\n";
		return rv;
	}
		
	rungroup.selectAll("rect.run")
		.map(function(d) { return this.parentNode.__data__; })
		.transition().duration(500)
		.attr("x", function(d) { return x(d.start); })
		.attr("width", function(d) { return x(d.stop-d.start); })
		.attr("height", y.rangeBand());

	rungroup.selectAll("rect.fill")
		.map(function(d) { return this.parentNode.__data__; })
		.transition().duration(500)
		.attr("x", function(d) { return x(d.fill); })
		.attr("width", function(d) { return x(d.stop-d.fill); })
		.attr("height", y.rangeBand());
		
	rungroup.selectAll("text")
		.map(function(d) { return this.parentNode.__data__; })
		.transition().duration(500)
		.attr("x", function(d) { return x(d.start); })
		.attr("y", y.rangeBand() / 2);
		
	rungroup.selectAll("title")
		.map(function(d) { return this.parentNode.__data__; })
		.text(tooltipfunc);
	
	// Now for all the new rungroups, create the active/postactive bars and the text field.
	var newrungroup = rungroup.enter()
		.append("g")
		.attr("class", "rungroup")
		.on("mousedown", selectFunc);
		
	newrungroup.append("svg:title")
		.text(tooltipfunc);
		
	newrungroup.append("rect")
		.attr("class", "run")
		.attr("x", function(d) { return x(d.start); })
		.attr("width", function(d) { return x(d.stop-d.start); })
		.attr("height", y.rangeBand())
		.on("mousedown", selectFunc);
	
	newrungroup.append("rect")
		.attr("class", "fill")
		.attr("x", function(d) { return x(d.fill); })
		.attr("width", function(d) { return x(d.stop-d.fill); })
		.attr("height", y.rangeBand());
	
	newrungroup.append("text")
		.attr("class", "value")
		.attr("x", function(d) { return x(d.start); })
		.attr("y", y.rangeBand() / 2)
		.attr("dx", 3)
		.attr("dy", ".35em")
		.text(function(d) { return d.descr; });
	
	tooltipfunc = function(d) {
		var rv = "reason:\t" + d.descr + "\n" +
			"begin:\t" + formatTime(d.start) + "\n" +
			"end:\t" + formatTime(d.stop) + "\n";
		return rv;
	}

	var stallgroup = bars.selectAll("g.stallgroup")
		.data(function(d) { if ("stalls" in d) return d.stalls; return []; });
		
	stallgroup.selectAll("rect.stall")
		.map(function(d) { return this.parentNode.__data__; })
		.transition().duration(500)
		.attr("x", function(d) { return x(d.start); })
		.attr("width", function(d) { return x(d.stop-d.start); })
		.attr("height", y.rangeBand()/6);
		
	stallgroup.selectAll("title")
		.map(function(d) { return this.parentNode.__data__; })
		.text(tooltipfunc);

	var newstallgroup = stallgroup.enter()
		.append("g")
		.attr("class", "stallgroup");
		
	stallgroup.exit().remove();
	
	newstallgroup.append("rect")
		.attr("class", "stall")
		.attr("x", function(d) { return x(d.start); })
		.attr("width", function(d) { return x(d.stop-d.start); })
		.attr("height", y.rangeBand()/6)
		.append("svg:title")
		.text(tooltipfunc);
	  
	
	// Finally setup the axes for the whole graph
	svg.select("g.x.axis")
		.call(xAxis);
	
	svg.select("g.y.axis")
		.call(yAxis);
}

regenGraph = function() {
	d3.text(dataFile, "text/json", function(text) {
		genGraph(JSON.parse(text))
		});
};

regenShowAndGraph = function() {
	for (var i=0; i < document.selections.show.length; i++) {
		showTypes[document.selections.show[i].value] = document.selections.show[i].checked;
	}
	regenGraph();
};

zoom = function(yfactor, xfactor) {
	h_scale *= yfactor;
	v_scale *= xfactor;
	regenGraph();
};

regenGraph();

var interval = setInterval("regenGraph()", 1000);
autoReloadChanged = function() {
	if (interval) clearInterval(interval);
	if (document.selections.autoreload.checked) {
		interval = setInterval("regenGraph()", 1000);
	}
}

loadData = function() {
	var data = JSON.parse(document.selections.inputdata.value);
	document.selections.autoreload.checked = false;
	autoReloadChanged();
	genGraph(data);
}

saveData = function() {
	var stringData = JSON.stringify(lastReadData);
	var uriContent = "data:application/json," + encodeURIComponent(stringData);
	var newWindow = window.open(uriContent, "Ambulant Visualizer JSON Data");
}
    </script>
  </body>
</html>
