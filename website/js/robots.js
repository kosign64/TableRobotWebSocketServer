"use strict";

/*
 * PROTOCOL
 *
 * Server sends:
 * JSON array of points' coordinates [x1, y1, x2, y2, ...]
 *
 * Server receives:
 * String containg robot numbers and control bytes: "number controlByte number controlByte..."
 * Example: "1 64 2 110"
 */

window.onload = function() {
	var numberParagraph = document.getElementById("robot_number");
	var canvas = document.getElementById("field");
	var ctx = canvas.getContext("2d");
	var robotNumber = 1;
	var keyboard = {
		up: false,
		down: false,
		left: false,
		right: false
	};

	var robotData = "1 64";


	var field = {
		X_OFFSET: 386,
        Y_OFFSET: 1,
        ACTUAL_WIDTH: 574 * (4 / 3),

        ROBOT_RADIUS: 60,
		POINT_RADIUS: 12,

		robots: [],

		drawBoard: function() {
			ctx.fillStyle = "red";
			ctx.fillRect(0, 0, canvas.width, canvas.height);
			ctx.fillStyle = "black";
			ctx.fillRect(this.boardOrigin.x, 
						 this.boardOrigin.y,
						 this.boardWidth, 
						 this.boardHeight);
		},

		drawRobots: function() {
			for(var i = 0; i < this.robots.length; ++i) {
				this.robots[i].center.x = this.robots[i].center.x *
					this.scaleFactor;
				this.robots[i].center.y = this.robots[i].center.y *
					this.scaleFactor;
				ctx.fillStyle = "blue";
				ctx.beginPath();
				ctx.arc(this.robots[i].center.x,
					    this.robots[i].center.y,
					    this.robotRadius / 2,
					    0, 2 * Math.PI);
				ctx.fill();

				ctx.fillStyle = "white";
				for(var j = 0; j < this.robots[i].points.length; ++j) {
					ctx.beginPath();
					this.robots[i].points[j].x = 
					    this.robots[i].points[j].x * this.scaleFactor;
					this.robots[i].points[j].y = 
				    	this.robots[i].points[j].y * this.scaleFactor;
					ctx.arc(this.robots[i].points[j].x,
					    	this.robots[i].points[j].y,
					    	this.pointRadius / 2,
					    	0, 2 * Math.PI);
					ctx.fill();
				}
			}
		},

		reDraw: function() {
			this.drawBoard();
			this.drawRobots();
		}
    };
	field.RECT_X0 = 692 - field.X_OFFSET;
	field.RECT_Y0 = 23 - field.Y_OFFSET;
	field.X_SCALE = 3746 / field.ACTUAL_WIDTH;
	field.RECT_WIDTH =  (3970 - field.X_OFFSET) - field.RECT_X0;
	field.RECT_HEIGHT =  (553 - field.Y_OFFSET) - field.RECT_Y0;
	field.scaleFactor = canvas.width / field.ACTUAL_WIDTH;
	field.boardOrigin = {
			x: (field.RECT_X0 / field.X_SCALE) * field.scaleFactor,
			y: field.RECT_Y0 * field.scaleFactor
	};
	field.boardWidth = field.RECT_WIDTH * field.scaleFactor / 
			field.X_SCALE;
	field.boardHeight = field.RECT_HEIGHT * field.scaleFactor;
	field.robotRadius = field.ROBOT_RADIUS * field.scaleFactor;
	field.pointRadius = field.POINT_RADIUS * field.scaleFactor;

	field.drawBoard();

	var wsUri = "ws://194.85.169.204:3337";
	var socket = new WebSocket(wsUri);
	socket.onmessage = getPoints;
	socket.onerror = function(evt) {
		alert('Error: ' + 'connection error');
	};

	function getPoints(evt) {
		var points = JSON.parse(evt.data);
		var points2D = [];
		for(var i = 0; i < points.length; i += 2) {
			points[i] = (points[i] - field.X_OFFSET) / field.X_SCALE;
			points[i + 1] -= field.Y_OFFSET;
			points2D.push(new Point2D(points[i], points[i + 1]));
		}
		field.robots = findRobots(points2D);
		//field.reDraw();

		socket.send(robotData);
	}

	function Point2D(x, y) {
		this.x = x;
		this.y = y;
	}

	function Robot(center, points) {
		this.center = center;
		this.points = points;
	}

	function length(p1, p2) {
		return Math.sqrt(Math.pow(p1.x - p2.x, 2) +
						 Math.pow(p1.y - p2.y, 2));
	}

	function center(p1, p2) {
		var x = (p1.x + p2.x) / 2;
		var y = (p1.y + p2.y) / 2;

		return new Point2D(x, y);
	}

	function findRobots(rawPoints) {
		// Get rid of non-robot points
		var points = [];
		for(var i = 0; i < rawPoints.length - 1; ++i) {
			var newPoint = true;
			for(var j = i + 1; j < rawPoints.length; ++j) {
				if(length(rawPoints[i], rawPoints[j]) < 10) {
					newPoint = false;
					break;
				}
			}
			if(newPoint) {
				points.push(rawPoints[i]);
			}
		}
		if(rawPoints.length > 0) {
			points.push(rawPoints[rawPoints.length - 1]);
		}

		// Find robots
		var robots = [];
		for(var i = 0; i < points.length - 1; ++i) {
			for(var j = i + 1; j < points.length; ++j) {
				if(length(points[i], points[j]) < 22) {
					var robotCenter = center(points[i], points[j]);
					var robotPoints = [points[i], points[j]];
					robots.push(new Robot(robotCenter, 
									      robotPoints));
				}
			}
		}

		return robots;
	}

	function setControl() {
		var control = 0;
		if(keyboard.up && !keyboard.left && !keyboard.right)
    	{
        	control = 118;
    	}
    	else if(keyboard.up && keyboard.left)
    	{
        	control = 117;
    	}
    	else if(keyboard.up && keyboard.right)
    	{
        	control = 110;
    	}
    	else if(keyboard.left)
    	{
        	control = 104;
    	}
    	else if(keyboard.right)
    	{
        	control = 69;
    	}
    	else if(keyboard.down)
    	{
        	control = 82;
    	}
    	else
    	{
        	control = 64;
    	}

    	robotData = robotNumber + " " + control;
	}

	this.onkeydown = function(evt) {
		// W - 87
		// A - 65
		// S - 83
		// D - 68
		// 1 - 49
		if(evt.keyCode > 48 && evt.keyCode < 58) {
			robotNumber = evt.keyCode - 48;
			numberParagraph.innerHTML = "Управление роботом №" + 
			robotNumber;
		} else {
			switch(evt.keyCode) {
				case 87:
					keyboard.up = true;
					break;

				case 65:
					keyboard.left = true;
					break;

				case 83:
					keyboard.down = true;
					break;

				case 68:
					keyboard.right = true;
					break;
			}
		}

		setControl();
	}

	this.onkeyup = function(evt) {
		switch(evt.keyCode) {
			case 87:
				keyboard.up = false;
				break;

			case 65:
				keyboard.left = false;
				break;

			case 83:
				keyboard.down = false;
				break;

			case 68:
				keyboard.right = false;
				break;
		}
		setControl();
	}

	setInterval(function(){field.reDraw();}, 25);

};
