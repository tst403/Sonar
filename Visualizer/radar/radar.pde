import processing.serial.*;
import java.util.Random;
import java.nio.ByteBuffer;

int heightVal;
int widthVal;
Match [] marks = new Match[181];
byte [] intBuffer = new byte[4];

//Display disp;
Serial arduino;
final String portName = "COM3";
final int MAX_RANGE = 40;

class Line {
  color c;
  PVector start;
  PVector end;

  public Line(PVector start, PVector end, color c) {
    this.start=start;
    this.end=end;
    this.c=c;
  }

  void sketch() {
    stroke(this.c);
    strokeWeight(2);
    line(start.x, start.y, end.x, end.y);
  }
}

void drawBorder() {
  stroke(color(0, 255, 0));
  strokeWeight(5);
  noFill();
  circle(center.x, center.y, widthVal);
}

class LineFactory {

  public LineFactory() {
  }

  public Line createLine(int value, color c, boolean vertical, boolean legend, int ind) {
    PVector start;
    PVector end;

    if (vertical) {
      start = new PVector(value, 0);
      fill(255);
      text("" + ind, start.x, start.y + 16);
      end = new PVector(value, heightVal);
    } else {
      start = new PVector(0, value);
      fill(255);
      text("" + ind, start.x, start.y);
      end = new PVector(widthVal, value);
    }

    return new Line(start, end, c);
  }
}

class Grid {
  int hLines, vLines;

  public Grid(int hLines, int vLines) {
    this.hLines=hLines;
    this.vLines=vLines;
  }

  public void sketch() {
    int hInterval, vInterval;
    int i = 1;

    hInterval = widthVal / this.hLines;
    vInterval = heightVal / this.vLines;

    LineFactory fac = new LineFactory();

    for (int x = 0; x < widthVal; x+= vInterval) {
      fac.createLine(x, color(0, 255, 0), true, true, i++).sketch();
    }
    i = 1;
    for (int y = 0; y < heightVal; y+= vInterval) {
      fac.createLine(y, color(0, 255, 0), false, true, i++).sketch();
    }
  }
}

class Servo {

  int angle = 0;
  boolean desc = false;

  public Servo() {
  }

  public int getPos() {
    if (this.angle == 180) {
      desc = true;
    } else if (this.angle == 0) {
      desc = false;
    }

    if (desc) {
      return this.angle--;
    } else {
      return this.angle++;
    }
  }

  public boolean isTarget() {
    Random rnd = new Random();
    return rnd.nextInt(100) <= 20;
  }
}

public class Match {
  private static final int SIZE = 25;
  private static final int TTL_MAX = 180;

  public PVector pos;
  private color col;
  private int ttl;

  public Match(PVector pos) {
    this.pos = pos;
    this.col = color(0, 255, 0, 255);
    this.ttl = TTL_MAX;
  }

  private void update() {
    this.ttl--;
    this.col = color(0, 255, 0, 255 - (TTL_MAX - ttl));
  }

  public void sketch() {
    noStroke();
    fill(this.col);
    circle(this.pos.x, this.pos.y, SIZE);
    update();
  }
} 

public static float degToRad(int deg) {
  return PI * (deg / 180f);
}

public static PVector getCircle(int angleDeg, int radius, PVector center) {
  float angle = degToRad(angleDeg);
  return new PVector(center.x + (int)(radius * cos(angle)), center.y - (int)(radius * (sin(angle))));
}

class Target {
  PVector pos;

  public Target(PVector pos) {
    this.pos=pos;
  }

  public void sketch() {
    fill(color(255, 0, 0));
    ellipse(this.pos.x, this.pos.y, 20, 20);
  }
}

public PVector calcPointFromMeasure(int angle, int distance) {
  int deltaX, deltaY;
  float rad;

  angle = 360 - angle;
  rad = degToRad(angle);

  deltaX = (int)(distance * Math.cos(rad));
  deltaY = (int)(distance * Math.sin(rad));

  return new PVector(center.x + deltaX, center.y + deltaY);
}

public PVector calcPointFromMeasureRel(int angle, int distance) {
  distance = (int)(((float)distance / MAX_RANGE) * (widthVal / 2));

  return calcPointFromMeasure(angle, distance);
}

int fromByteArray(byte[] bytes) {
  return ByteBuffer.wrap(bytes).getInt();
}

int[] parseData(int angle) {
  int[] res = new int[2];

  try {
    intBuffer = arduino.readBytes(4);
    int a = fromByteArray(intBuffer);
    res[0] = a;


    intBuffer = arduino.readBytes(4);
    a = fromByteArray(intBuffer);
    res[1] = a;

    return res;
  }
  catch(Exception e) {
    res[0] = angle;
    res[1] = -1;
    return res;
  }
}

Grid g;
int radius;
PVector center;
PVector lineEdge;
Servo servo;

/*public class Display {
 
 private int angle;
 private int distance;
 
 public Display() {
 angle = 0;
 }
 
 public void sketch() {
 
 
 public void update()
 }
 }*/

public void iterateMarks() {
  for (int i = 0; i < marks.length; i++) {
    if (marks[i] != null) {
      marks[i].sketch();
    }
  }
}

// [ ================================ SETUP ================================ ]

void setup() {
  background(0);
  fullScreen();

  heightVal = height;
  widthVal = width;


  g = new Grid(20, 15);
  radius = max(heightVal, widthVal);
  center = new PVector(widthVal / 2, heightVal);
  servo = new Servo();
  arduino = new Serial(this, portName, 9600);
  delay(2000);
  arduino.write("" + MAX_RANGE +"\n");
  //disp = new Display();
}

// [ ================================ DRAW ================================ ]

void draw() {
  if (arduino.available() > 0) {

    int angle, dist;
    angle = 0;
    dist = -1;

    String res = arduino.readStringUntil(10);
    if (res != null) {
      try {
        String[] delimeted = res.split("@");
        angle = parseInt(delimeted[0]);
        dist = parseInt(delimeted[1]);
      }
      catch(Exception e) {
        println("error");
        return;
      }

      background(0);
      g.sketch();

      lineEdge = getCircle(angle, /*radius*/ widthVal / 2, center);
      stroke(color(50, 255, 50));
      strokeWeight(2);
      line(center.x, center.y, lineEdge.x, lineEdge.y);

      drawBorder();

      if (dist != -1) {
        PVector temp = calcPointFromMeasureRel(angle, dist);
        marks[angle] = new Match(temp);
      }

      if (marks[angle] != null && marks[angle].ttl <= 0) {
        marks[angle] = null;
      }


      iterateMarks();
    }
  }
}
