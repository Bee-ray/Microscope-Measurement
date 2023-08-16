import tkinter as tk
import tkinter.filedialog
from PIL import Image, ImageTk
from math import sqrt



class Application():

    def __init__(self,windowname='',size=(1280,960),xresizable=False,yresizable=False):
        self._root = tk.Tk()
        self._root.title(windowname)
        self._size = size
        self._root.geometry(str(self._size[0])+'x'+str(self._size[1]))
        self._root.resizable(xresizable,yresizable)

        self._frame_config = tk.Frame(self._root, width = 1280, height = 20)
        self._frame_canvas = tk.Frame(self._root, width = 1280, height = size[1]-20)

        self._mode = tk.IntVar() # 0 is contrast mode, 1 is distance mode

        self._ori_photo=None
        self._photo = None
        self._canvas = None
        
        self._point1=None
        self._point2=None
        self._point3=None
        self._point4=None

        self._rec1 = None
        self._rec2 = None
        self._line = None

        self._diffR = tk.IntVar()
        self._diffG = tk.IntVar()
        self._diffB = tk.IntVar()

        self._dist = tk.DoubleVar()

        self._magnification = ['2x','10x','20x','50x','100x']

        self._mag = tk.IntVar()
        self._magtext = tk.StringVar()
        self._magtext.set(self._magnification[self._mag.get()])


        self._calibration = [1./2,1./10,1./20,1./50,1./100]
        try:
            with open('calibration.txt') as f_obj:
                self._calibration = []
                while True:
                    a = f_obj.readline()
                    if a == '':
                        break
                    else:
                        self._calibration.append(float(a))

        except FileNotFoundError:
            pass
    
        self.display()

    
    def display(self):
        self._radio_buttons()
        self._output()
        self._open_buttons()
        self._make_canvas()

        self._frame_config.grid(row = 0, column = 0)
        self._frame_canvas.grid(row = 1, column = 0,pady = 10)
        self._root.mainloop()
    
    def _radio_buttons(self):

        tk.Scale(self._frame_config, from_=0,to = 4,variable = self._mag,orient = tk.HORIZONTAL,showvalue = False,command=self._change_magnification,sliderrelief=tk.FLAT,font=('Arial',12)).grid(row=0,column=9)
        tk.Label(self._frame_config, textvariable= self._magtext,width = 5,font=('Arial',12)).grid(row=0,column=10)

        tk.Radiobutton(self._frame_config,text='Contrast', variable = self._mode, value = 0,command = self._canvas_bind_event,font=('Arial',12)).grid(row=0,column=1)
        tk.Radiobutton(self._frame_config,text='Distance', variable = self._mode, value = 1,command = self._canvas_bind_event,font=('Arial',12)).grid(row=0,column=8)

    def _output(self):
        textbox_diffr = tk.Entry(self._frame_config,textvariable = self._diffR, state='readonly',font=('Arial',12))
        label_diffr = tk.Label(self._frame_config,text = 'R:',width = 3,font=('Arial',12))
        textbox_diffg = tk.Entry(self._frame_config,textvariable = self._diffG, state='readonly',font=('Arial',12))
        label_diffg = tk.Label(self._frame_config,text = 'G:',width = 3,font=('Arial',12))
        textbox_diffb = tk.Entry(self._frame_config,textvariable = self._diffB, state='readonly',font=('Arial',12))
        label_diffb = tk.Label(self._frame_config,text = 'B:',width = 3,font=('Arial',12))

        label_diffr.grid(row=0,column= 2 )
        textbox_diffr.grid(row = 0,column = 3)
        label_diffg.grid(row=0,column=4)
        textbox_diffg.grid(row = 0, column = 5)
        label_diffb.grid(row=0,column=6)
        textbox_diffb.grid(row = 0, column = 7)


        label_dist = tk.Label (self._frame_config, text = 'um',font=('Arial',12))
        textbox_dist= tk.Entry(self._frame_config, textvariable=self._dist, state='readonly',font=('Arial',12))

        label_dist.grid(row=0,column=12)
        textbox_dist.grid(row=0,column=11)

    def _open_buttons(self):
        button = tk.Button(self._frame_config, text='Open',command=self._open_image,font=('Arial',12))
        button.grid(row = 0, column =0)

    def _open_image(self):
        path = tk.filedialog.askopenfilename(title='Open File')
        if path:
            self._ori_photo=Image.open(path)
            x0, y0, width, height = self._ori_photo.getbbox()
            self._ori_photo = self._ori_photo.resize((1200,int(height*1200/width)))
            self._photo=ImageTk.PhotoImage(self._ori_photo)
            self._canvas.create_image(0,0,anchor=tk.NW,image = self._photo)
            self._canvas_bind_event()

    def _make_canvas(self):
        self._canvas=tk.Canvas(self._frame_canvas,width=1200,height=900)
        self._canvas.grid()

    def _canvas_bind_event(self):
        self._point1 = None
        self._point2 = None
        self._point3 = None
        self._point4 = None
        self._canvas.delete(self._rec1)
        self._canvas.delete(self._rec2)
        self._canvas.delete(self._line)
        self._diffR.set(0)
        self._diffG.set(0)
        self._diffB.set(0)
        self._dist.set(0)
        self._rec1 = None
        self._rec2 = None
        self._line = None

        if self._mode.get()==0:
            self._canvas.bind('<Button-1>',self._click_contrast)
        if self._mode.get()==1:
            self._canvas.bind('<Button-1>',self._click_distance)

    def _click_distance(self,event):
        if self._photo:
            if self._point2:
                self._point1 = None
                self._point2 = None
                self._canvas.delete(self._line)
                self._line = None
            if not self._point1:
                self._point1 = (event.x,event.y)
            elif not self._point2:
                self._point2 = (event.x,event.y)
                self._line = self._canvas.create_line(self._point1[0],self._point1[1],self._point2[0],self._point2[1])
                
            if self._point1 and self._point2:
                self._dist.set(round(sqrt((self._point1[0]-self._point2[0])**2+(self._point1[1]-self._point2[1])**2)*self._calibration[self._mag.get()],8))

    def _change_magnification(self,mag):
         self._magtext.set(self._magnification[self._mag.get()])
         if self._point1 and self._point2:
                self._dist.set(round(sqrt((self._point1[0]-self._point2[0])**2+(self._point1[1]-self._point2[1])**2)*self._calibration[self._mag.get()],8))
       

    def _click_contrast(self,event):
        if self._photo:
            if self._point4:
                self._point1 = None
                self._point2 = None
                self._point3 = None
                self._point4 = None
                self._canvas.delete(self._rec1)
                self._canvas.delete(self._rec2)
                self._rec1 = None
                self._rec2 = None

            if not self._point1:
                self._point1 = (event.x,event.y)
            elif not self._point2:
                self._point2 = (event.x,event.y)
                self._rec1 = self._canvas.create_rectangle(self._point1[0],self._point1[1],self._point2[0],self._point2[1],fill='')

            elif not self._point3:
                self._point3 = (event.x,event.y)
            elif not self._point4:
                self._point4 = (event.x,event.y)
                self._rec2 = self._canvas.create_rectangle(self._point3[0],self._point3[1],self._point4[0],self._point4[1],fill='')
                self._differenceRGB()

    def _differenceRGB(self):
        xrange1 = range(min(self._point1[0],self._point2[0]),max(self._point1[0],self._point2[0])+1)
        yrange1 = range(min(self._point1[1],self._point2[1]),max(self._point1[1],self._point2[1])+1)
        xrange2 = range(min(self._point3[0],self._point4[0]),max(self._point3[0],self._point4[0])+1)
        yrange2 = range(min(self._point3[1],self._point4[1]),max(self._point3[1],self._point4[1])+1)

        avgR1 = 0
        avgG1 = 0
        avgB1 = 0
        avgR2 = 0
        avgG2 = 0
        avgB2 = 0

        rgb = self._ori_photo.convert('RGB')

        for x1 in xrange1:
            for y1 in yrange1:
                avgR1 += rgb.getpixel((x1,y1))[0]
                avgG1 += rgb.getpixel((x1,y1))[1]
                avgB1 += rgb.getpixel((x1,y1))[2]
        try:
            avgR1 = avgR1 / (len(xrange1)*len(yrange1))
            avgG1 = avgG1 / (len(xrange1)*len(yrange1))
            avgB1 = avgB1 / (len(xrange1)*len(yrange1))
        except ZeroDivisionError:
            avgR1 = 0
            avgG1 = 0
            avgB1 = 0
        for x2 in xrange2:
            for y2 in yrange2:
                avgR2 += rgb.getpixel((x2,y2))[0]
                avgG2 += rgb.getpixel((x2,y2))[1]
                avgB2 += rgb.getpixel((x2,y2))[2]
        try:    
            avgR2 = avgR2 / (len(xrange2)*len(yrange2))
            avgG2 = avgG2 / (len(xrange2)*len(yrange2))
            avgB2 = avgB2 / (len(xrange2)*len(yrange2))
        except ZeroDivisionError:
            avgR2 = 0
            avgG2 = 0
            avgB2 = 0
        try:
            self._diffR.set(round(2*(avgR2-avgR1)/(avgR2+avgR1),8))
            self._diffG.set(round(2*(avgG2-avgG1)/(avgG2+avgG1),8))
            self._diffB.set(round(2*(avgB2-avgB1)/(avgB2+avgB1),8))
        except ZeroDivisionError:
            self._diffR.set(0)
            self._diffG.set(0)
            self._diffB.set(0)
        

app = Application("Microscope Measurement")
