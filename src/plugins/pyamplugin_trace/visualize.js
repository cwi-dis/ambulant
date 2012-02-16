// Semi-constants.

var formatTime = d3.format(".3f");	// How to format times
var m = [30, 10, 10, 330],			// Margins and default width and height of the graph
    w = 960 - m[1] - m[3],
    h = 930 - m[0] - m[2];

var dataFile="data.json";	// Relative URL of the datafile. Shared with pyamplugin_trace/tracer.py.
var lastReadData = [];		// Copy of last read data, for saving.

// Global variables.
var svg = null;		// Will hold the svg element, set by initVisualize().
var background = null;	// Will hold the background placeholder.
var selectHelpers = null; // Will hold foreground selection helper graphics.

var h_scale = 1.0;	// Current horizontal scale factor.
var v_scale = 1.0;	// Current vertical scale factor.

// XXX var format = d3.format(",.1f");

// The scales and axes. Note that these are functions!

var x = d3.scale.linear().range([0, w*h_scale]);
var y = d3.scale.ordinal().rangeRoundBands([0, h*v_scale], .1);

var xAxis = d3.svg.axis().scale(x).orient("top").tickSize(-h);
var yAxis = d3.svg.axis().scale(y).orient("left").tickSize(0);

// Datastructure for which items to show. Mirrors "selections" form in visualize.html.
var showTypes = new Object();
showTypes["medianode"] = true;
showTypes["prefetch"] = true;
showTypes["playable"] = true;
showTypes["bandwidth"] = true;
showTypes["*"] = true;

var interval = null;	// Will be set to auto-recurring reload

// Initialize the visualizer. 
var initVisualize = function() {

	// Create the main SVG object and the placeholders for the axes
	svg = d3.selectAll("div#graphlocation").append("svg")
	.attr("width", w*h_scale + m[1] + m[3])
	.attr("height", h*v_scale + m[0] + m[2])
	.append("g")
	.attr("transform", "translate(" + m[3] + "," + m[0] + ")");

	background = svg.append("g")
		.attr("class", "background");
		
	svg.append("g")
		.attr("class", "x axis");
	
	svg.append("g")
		.attr("class", "y axis");
		

	// Framework complete. Fill in the data.
	regenGraph();

	selectionHelpers = svg.append("g")
		.attr("class", "selectionHelpers");
		
	visualizerAutoReloadChanged();
}

// Function that loads JSON data and regenerates the graph.
var regenGraph = function() {
	d3.text(dataFile, "text/json", function(text) {
		genGraph(JSON.parse(text))
		});
};

// Called (from html form) when visualizer options have changed. Re-display the data.
var visualizerOptionsChanged = function() {
	for (var i=0; i < document.selections.show.length; i++) {
		showTypes[document.selections.show[i].value] = document.selections.show[i].checked;
	}
	regenGraph();
};

// Called (from html form) for implementaing h/v zooming.
var visualizerZoom = function(yfactor, xfactor) {
	h_scale *= yfactor;
	v_scale *= xfactor;
	regenGraph();
};

// Function to implement changing the autoreload flag.
var visualizerAutoReloadChanged = function() {
	if (interval) clearInterval(interval);
	interval = null;
	if (document.selections.autoreload.checked) {
		interval = setInterval("regenGraph()", 1000);
	}
}

// Load visualizer JSON data from HTML form text field.
var loadData = function() {
	var data = JSON.parse(document.selections.inputdata.value);
	document.selections.autoreload.checked = false;
	autoReloadChanged();
	genGraph(data);
}

// Save data to a new window.
var saveData = function() {
	var stringData = JSON.stringify(lastReadData);
	var uriContent = "data:application/json," + encodeURIComponent(stringData);
	var newWindow = window.open(uriContent, "Ambulant Visualizer JSON Data");
}

// Function that handles the global aspects of a node being selected.
var prepareForSelect = function() {
	// Deselect old selection
	svg.selectAll(".selected")
		.classed("selected", 0);
	
	// Select the currrent run rectangle
	d3.select(this).selectAll("rect.run")
		.classed("selected", 1);
	d3.select(this).selectAll("line.guideline")
		.classed("selected", 1);
	
};

// Function that updates (or creates) the graph with new data.
genGraph = function(data) {
	// Keep the data (for saving)
	lastReadData = data;
	
	// Filter the data on the nodes that we want to visualize
	var nodeData = [];
	var bandwidthData = [];
	var is_odd = false;
	for( var dataItemIndex in data) {
		var dataItem = data[dataItemIndex];
		var objType = dataItem.objtype;
		if (objType in showTypes) {
			if (showTypes[objType]) {
				if (dataItem.objtype != "playable") is_odd = !is_odd;
				dataItem.odd = is_odd;
				nodeData.push(dataItem);
			}
		} else {
			if (showTypes["*"]) {
				if (dataItem.objtype != "playable") is_odd = !is_odd;
				dataItem.odd = is_odd;
				nodeData.push(dataItem);
			}
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
				bandwidthData.unshift(newbw);
			}
		}
	}
	var allData = [];	
	allData = allData.concat(bandwidthData, nodeData);
		
	// Set the scale domain and ranges
	x.domain([0, d3.max(allData, function(d) { return d.runs.length ? d.runs[0].stop : 0; })]);
	y.domain(allData.map(function(d) { return d.objid; }));
	var new_h = allData.length * 3 * 10; // Number(getComputedStyle('graphlocation', '').fontSize.match(/(\d+)px/)[1]);
	x.range([0, w*h_scale]);
	y.rangeRoundBands([0, new_h*v_scale], .1);
	
	// Get the graph, and set height and width
	d3.selectAll("div#graphlocation svg")
		.transition().duration(500)
		.attr("width", w*h_scale + m[1] + m[3])
		.attr("height", new_h*v_scale + m[0] + m[2]);
	
	// Helper function: return the line() object given a d.stripdata object.
	var stripDataFunc = function(d) {
		var maxy = d3.max(d.stripdata, function(d) { return d[1]; });
		var line = d3.svg.line()
			.x(function(d) { return x(d[0]); })
			.y(function(d) { return y.rangeBand() * (d[1]/maxy); });
		return line(d.stripdata);
	};
	
	// Set up the bandwidth indicators
	var setupGlobalBandwidth = function(data) {
		var bwgroup = svg.selectAll("g.bandwidth")
		.data(data, function(d) { return d.objid; });
		
		bwgroup.exit().remove();
		
		bwgroup.enter().append("g")
			.attr("class", "bandwidth")
			.attr("transform", function(d) { return "translate(0," + y(d.objid) + ")"; })
			.append("path")
			.attr("d", stripDataFunc);
			
		bwgroup.select("path")
			.attr("d", stripDataFunc);
	};
	
	setupGlobalBandwidth(bandwidthData);
			
	// Setup the background.
	var setupBackground = function(data) {
		var bgbars = background.selectAll("rect.background")
			.data(data, function(d) { return d.objid; } );
			
		bgbars.select("rect")
			.classed("oddbackground", function(d) { return d.odd; })
			.attr("x", -m[3])
			.attr("y", function(d) { return y(d.objid); })
			.attr("width", w*h_scale + m[3])
			.attr("height", y.rangeBand()*1.1);

		bgbars.exit().remove();
		
		bgbars.enter().append("rect")
			.attr("class", "background")
			.classed("oddbackground", function(d) { return d.odd; })
			.attr("x", -m[3])
			.attr("y", function(d) { return y(d.objid); })
			.attr("width", w*h_scale + m[3])
			.attr("height", y.rangeBand()*1.1);
	}
	
	setupBackground(nodeData);
	
	// Now set up the bars (rows). Do this separately for existing/new/deleted data.
	
	var setupBars = function(data) {
		var bars = svg.selectAll("g.bar")
			.data(data, function(d) { return d.objid; })
		
		bars.transition().duration(500)
			.attr("transform", function(d) { return "translate(0," + y(d.objid) + ")"; });
	
		bars.enter().append("g")
			.attr("class", "bar")
			.classed("medianode", function(d) { return d.objtype == "medianode"; })
			.classed("playable", function(d) { return d.objtype == "playable"; })
			.classed("prefetch", function(d) { return d.objtype == "prefetch"; })
			.attr("transform", function(d) { return "translate(0," + y(d.objid) + ")"; });
			
		bars.exit().remove();
		
		return bars;
	}
	
	var bars = setupBars(nodeData);

	// Set up the grouped data (runs) within a horizontal bar.
	
	var setupRungroup = function(bars, tooltipfunc) {
		var rungroup = bars.selectAll("g.rungroup")
			.data(function(d) {return d.runs; });
		
		rungroup.exit().remove();
	
		// Now modify the rects in the existing rungroups
		rungroup.select("rect.run")
			.transition().duration(500)
			.attr("x", function(d) { return x(d.start); })
			.attr("width", function(d) { return x(d.stop-d.start); })
			.attr("height", y.rangeBand());
	
		rungroup.select("rect.fill")
			.transition().duration(500)
			.attr("x", function(d) { return x(d.fill); })
			.attr("width", function(d) { return x(d.stop-d.fill); })
			.attr("height", y.rangeBand());
			
		rungroup.select("text")
			.transition().duration(500)
			.attr("x", function(d) { return x(d.start); })
			.attr("y", y.rangeBand() / 2);
			
		rungroup.select("title")
			.text(tooltipfunc);
		
		rungroup.select("line.guidelineStart")
			.transition().duration(500)
			.attr('x1', function(d) { return x(d.start); })
			.attr('x2', function(d) { return x(d.start); })
			.attr('y1', function(d) { return -y(this.parentNode.parentNode.__data__.objid); })
			.attr('y2', h);
		rungroup.select("line.guidelineFill")
			.transition().duration(500)
			.attr('x1', function(d) { return x(d.fill); })
			.attr('x2', function(d) { return x(d.fill); })
			.attr('y1', function(d) { return -y(this.parentNode.parentNode.__data__.objid); })
			.attr('y2', h);
		rungroup.select("line.guidelineStop")
			.transition().duration(500)
			.attr('x1', function(d) { return x(d.stop); })
			.attr('x2', function(d) { return x(d.stop); })
			.attr('y1', function(d) { return -y(this.parentNode.parentNode.__data__.objid); })
			.attr('y2', h);

		// Now for all the new rungroups, create the active/postactive bars and the text field.
		var newrungroup = rungroup.enter()
			.append("g")
			.attr("class", "rungroup")
			.on("mousedown", prepareForSelect);
			
		newrungroup.append("svg:title")
			.text(tooltipfunc);
			
		newrungroup.append("rect")
			.attr("class", "run")
			.attr("x", function(d) { return x(d.start); })
			.attr("width", function(d) { return x(d.stop-d.start); })
			.attr("height", y.rangeBand())
			.on("mousedown", prepareForSelect);
		
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

		newrungroup.append("line")
			.attr("class", "guideline guidelineStart")
			.attr('x1', function(d) { return x(d.start); })
			.attr('x2', function(d) { return x(d.start); })
			.attr('y1', function(d) { return -y(this.parentNode.parentNode.__data__.objid); })
			.attr('y2', h);
		newrungroup.append("line")
			.attr("class", "guideline guidelineFill")
			.attr('x1', function(d) { return x(d.fill); })
			.attr('x2', function(d) { return x(d.fill); })
			.attr('y1', function(d) { return -y(this.parentNode.parentNode.__data__.objid); })
			.attr('y2', h);
		newrungroup.append("line")
			.attr("class", "guideline guidelineStop")
			.attr('x1', function(d) { return x(d.stop); })
			.attr('x2', function(d) { return x(d.stop); })
			.attr('y1', function(d) { return -y(this.parentNode.parentNode.__data__.objid); })
			.attr('y2', h);
	}
	
	var tooltipfunc = function(d) {
		var rv = "node: \t" + d.descr + "\n\n" +
			"start:\t" + formatTime(d.start) + "\n";
		if (d.fill) rv += "fill:\t\t" + formatTime(d.fill) + "\n";
		rv += "stop: \t" + formatTime(d.stop) + "\n";
		return rv;
	}

	setupRungroup(bars, tooltipfunc);
			
	// Setup the indicators for the stalls in the playables.
	
	var setupStalls = function(bars, tooltipfunc) {
		var stallgroup = bars.selectAll("g.stallgroup")
			.data(function(d) { if ("stalls" in d) return d.stalls; return []; });
			
		stallgroup.select("rect.stall")
			.transition().duration(500)
			.attr("x", function(d) { return x(d.start); })
			.attr("width", function(d) { return x(d.stop-d.start); })
			.attr("height", y.rangeBand()/6);
			
		stallgroup.select("title")
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
	}
	  
	tooltipfunc = function(d) {
		var rv = "reason:\t" + d.descr + "\n" +
			"begin:\t" + formatTime(d.start) + "\n" +
			"end:\t" + formatTime(d.stop) + "\n";
		return rv;
	}

	setupStalls(bars, tooltipfunc);
	
	// Finally setup the axes for the whole graph
	svg.select("g.x.axis")
		.call(xAxis);
	
	svg.select("g.y.axis")
		.call(yAxis);
}

