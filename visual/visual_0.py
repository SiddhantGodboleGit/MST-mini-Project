#!/usr/bin/env python3
import tkinter as tk
from tkinter import ttk
from PIL import Image, ImageTk
import os

# Ensure the script is run from the correct directory
script_dir = os.path.dirname(os.path.abspath(__file__))
os.chdir(script_dir)
print("Current working directory:", os.getcwd())

class GraphPropertiesForm:
    def __init__(self, master):
        self.master = master
        master.title("Graph Properties")
        master.geometry("350x670")
        master.resizable(False, False)

        bg_image_file = Image.open("visual/visual.png")
        bg_image_file = bg_image_file.resize((380, 680), Image.Resampling.LANCZOS)
        self.bg_image = ImageTk.PhotoImage(bg_image_file)
        background_label = tk.Label(master, image=self.bg_image)
        background_label.place(x=0, y=0, relwidth=1, relheight=1)

        entry_bg = "#ffffff"
        entry_font = ('Helvetica', 15)
        entry_width = 10
        entry_relief = tk.SOLID
        entry_borderwidth = 0

        self.cores_var = tk.StringVar(value="4")
        cores_entry = tk.Entry(master, textvariable=self.cores_var, font=entry_font, width=entry_width,
                       relief=entry_relief,  highlightthickness=0)
        cores_entry.place(x=160, y=55)

        self.seed_var = tk.StringVar(value="0")
        seed_entry = tk.Entry(master, textvariable=self.seed_var, font=entry_font, width=entry_width, justify='right',
                      bg=entry_bg, relief=entry_relief, borderwidth=entry_borderwidth, highlightthickness=0, disabledbackground='#fff')
        seed_entry.place(x=160, y=113)

        # Nodes Entry
        self.nodes_var = tk.StringVar(value="64")
        nodes_entry = tk.Entry(master, textvariable=self.nodes_var, font=entry_font, width=entry_width, justify='right',
                      bg=entry_bg, relief=entry_relief, borderwidth=entry_borderwidth, highlightthickness=0, disabledbackground='#fff')
        nodes_entry.place(x=160, y=175)

        # Edges Entry
        self.edges_var = tk.StringVar(value="256")
        edges_entry = tk.Entry(master, textvariable=self.edges_var, font=entry_font, width=entry_width, justify='right',
                      bg=entry_bg, relief=entry_relief, borderwidth=entry_borderwidth, highlightthickness=0, disabledbackground='#fff')
        edges_entry.place(x=160, y=237)

        # Density Entry
        self.density_var = tk.StringVar(value="0.05")
        density_entry = tk.Entry(master, textvariable=self.density_var, font=entry_font, width=entry_width, justify='right',
                      bg=entry_bg, relief=entry_relief, borderwidth=entry_borderwidth, highlightthickness=0, disabledbackground='#fff')
        density_entry.place(x=160, y=299)

        # --- Checkboxes ---
        self.planar_var = tk.BooleanVar()
        planar_check = tk.Checkbutton(master, variable=self.planar_var, bg='#f0f0f0',
               activebackground="#ffffff", activeforeground="#000000", background="#f0f0f0", indicatoron=False,
               selectcolor="#77ccff",height=2, width=5,highlightbackground="#f0f0f0", highlightthickness=3)
        planar_check.place(x=195, y=347)

        self.directed_var = tk.BooleanVar()
        directed_check = tk.Checkbutton(master, variable=self.directed_var, bg='#f0f0f0',
               activebackground="#ffffff", activeforeground="#000000", background="#f0f0f0", indicatoron=False,
               selectcolor="#77ccff",height=2, width=5,highlightbackground="#f0f0f0", highlightthickness=3)
        directed_check.place(x=195, y=409)

        self.connected_var = tk.BooleanVar(value=True)
        connected_check = tk.Checkbutton(master, variable=self.connected_var, bg='#f0f0f0',
               activebackground="#ffffff", activeforeground="#000000", background="#f0f0f0", indicatoron=False,
               selectcolor="#77ccff",height=2, width=5,highlightbackground="#f0f0f0", highlightthickness=3)
        connected_check.place(x=195, y=471)

        # --- Buttons ---
        self.submit_btn = tk.Button(master, text="Submit", command=self.submit_action, font=('Helvetica', 10, 'bold'), relief=tk.RIDGE, borderwidth=0,
                                    padx=19, pady=17, bg='#ddd', activeforeground='#000000')
        self.submit_btn.place(x=80, y=547)

        self.confirm_btn = tk.Button(master, text="Confirm", command=self.confirm_action, font=('Helvetica', 10, 'bold'), relief=tk.RIDGE, borderwidth=0,
                                    padx=19, pady=17, )
        self.confirm_btn.place(x=200, y=596)
        self.submit_btn.config(state=tk.NORMAL)
        self.confirm_btn.config(state=tk.DISABLED)

    def submit_action(self):
        self.submit_btn.config(state=tk.DISABLED)
        self.confirm_btn.config(state=tk.NORMAL)
        # Disable all entries and checkboxes
        for widget in self.master.winfo_children():
            if isinstance(widget, tk.Entry) or isinstance(widget, tk.Checkbutton):
                widget.config(state=tk.DISABLED)


    def confirm_action(self):
        data = {
            "seed": self.seed_var.get() or "0",
            "nodes": self.nodes_var.get() or "64",
            "edges": self.edges_var.get() or "256",
            "density": self.density_var.get() or "0.05",
            "planar": self.planar_var.get() if self.planar_var.get() is not None else False,
            "directed": self.directed_var.get() if self.directed_var.get() is not None else False,
            "connected": self.connected_var.get() if self.connected_var.get() is not None else True
        }
        print("Params:", data)
        #save to file
        with open("input_params.txt", "w") as f:
            f.write(str(data))
        self.submit_btn.config(state=tk.NORMAL)
        self.confirm_btn.config(state=tk.DISABLED)

if __name__ == "__main__":
    root = tk.Tk()
    app = GraphPropertiesForm(root)
    root.mainloop()