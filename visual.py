#!/usr/bin/env python3
import tkinter as tk
from tkinter import ttk
from PIL import Image, ImageTk
import subprocess
import os


# Ensure the current working directory is set to the script's directory
script_dir = os.path.dirname(os.path.abspath(__file__))
os.chdir(script_dir)    


class GraphPropertiesForm:
    def __init__(self, master):
        self.master = master
        self.mst_process = None
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
               activebackground="#ffffff", activeforeground="#000000", indicatoron=False,
               selectcolor="#77ccff",height=2, width=5,highlightbackground="#f0f0f0", highlightthickness=3)
        planar_check.place(x=195, y=347)

        self.directed_var = tk.BooleanVar()
        directed_check = tk.Checkbutton(master, variable=self.directed_var, bg='#f0f0f0',
               activebackground="#ffffff", activeforeground="#000000",  indicatoron=False,
               selectcolor="#77ccff",height=2, width=5,highlightbackground="#f0f0f0", highlightthickness=3)    
        directed_check.place(x=195, y=409)

        self.connected_var = tk.BooleanVar(value=True)
        connected_check = tk.Checkbutton(master, variable=self.connected_var, bg='#f0f0f0',
               activebackground="#ffffff", activeforeground="#000000",  indicatoron=False,
               selectcolor="#77ccff",height=2, width=5,highlightbackground="#f0f0f0", highlightthickness=3)
        connected_check.place(x=195, y=471)

        # --- Buttons ---
        #make a box

        self.submit_btn = tk.Button(master, text="Submit", command=self.submit_action, font=('Helvetica', 10, 'bold'), relief=tk.RIDGE, borderwidth=0,
                                    padx=19, pady=17, bg='#ddd', activeforeground='#000000')
        self.submit_btn.place(x=80, y=547)

        self.confirm_btn = tk.Button(master, text="",  font=('Helvetica', 10, 'bold'), relief=tk.RIDGE, borderwidth=0,
                                    padx=19, pady=17, width=9, bg='#ddd', activeforeground='#000000',background="#ddd")
        self.confirm_btn.place(x=200, y=596)
        self.submit_btn.config(state=tk.NORMAL)
        self.confirm_btn.config(state=tk.DISABLED)
        

    def submit_action(self):
        self.submit_btn.config(state=tk.DISABLED)
        # Disable all entries and checkboxes
        for widget in self.master.winfo_children():
            if isinstance(widget, tk.Entry):
                widget.config(state=tk.DISABLED)
            elif isinstance(widget, tk.Checkbutton):
                widget.config(state=tk.DISABLED)
        # Collect data from the form
        self.data = {
            "success": "1",
            "seed": self.seed_var.get() if self.seed_var.get() else "0",
            "nodes": self.nodes_var.get() if self.nodes_var.get() else "64",
            "edges": self.edges_var.get()  if self.edges_var.get() else "256",
            "density": self.density_var.get() if self.density_var.get() else "0.05",
            "planar": self.planar_var.get(),
            "directed": self.directed_var.get(),
            "connected": self.connected_var.get()
        }
        # Ckeck for existance of input_graph.txt
        if not os.path.exists("input_params.txt"):
            self.data["success"] = "0"
            self.confirm_btn.config(state=tk.NORMAL, text="Create", command=self.create_action)
        else:
            #check if matrix has same meta data with success = 1 and other parameters same
            with open("input_params.txt", "r") as f:
                existing_data = f.read()
                if existing_data.strip() == "\n".join(f"{key}: {value}" for key, value in self.data.items()):
                    self.confirm_btn.config(state=tk.NORMAL, text="MST", command=self.confirm_action)
                else:
                    self.data["success"] = "0"
                    self.confirm_btn.config(state=tk.NORMAL, text="Create", command=self.create_action)
                    

    def create_action(self):
        formatted_data = "\n".join(f"{key}: {value}" for key, value in self.data.items())
        with open("input_params.txt", "w") as f:
            f.write(formatted_data)
        self.confirm_btn.config(state=tk.DISABLED, text="creating...", command=self.confirm_action)
        # Wait until the existing data in the file has success == 1
        # while True:
        #     if os.path.exists("input_params.txt"):
        #         with open("input_params.txt", "r") as f:
        #             existing_data = f.read()
        #             if "success: 1" in existing_data:
        #                 break
        #     self.master.update_idletasks()
        #     self.master.after(100)

        result = subprocess.run(["make", "matrix"], cwd=script_dir, capture_output=False, text=True, check=True)

        self.confirm_btn.config(state=tk.NORMAL, text="MST", command=self.confirm_action)


    def confirm_action(self):
        self.submit_btn.config(state=tk.NORMAL)
        self.confirm_btn.config(state=tk.DISABLED)
        # reactivate all entries and checkboxes
        #kill all other processes
        if self.mst_process:
            self.mst_process.terminate()
            self.mst_process = None
        for widget in self.master.winfo_children():
            if isinstance(widget, tk.Entry):
                widget.config(state=tk.NORMAL)
            elif isinstance(widget, tk.Checkbutton):
                widget.config(state=tk.NORMAL)
        self.confirm_btn.config(text="MST_ing")
        print("Get MST")
        self.mst_process = subprocess.Popen(["make", "mst"], cwd=script_dir)
        
    def on_closing(self):
        # Kill the subprocess when the main window is closed
        if self.mst_process and self.mst_process.poll() is None:
            self.mst_process.kill()
        self.master.destroy()


if __name__ == "__main__":
    root = tk.Tk()
    app = GraphPropertiesForm(root)
    root.protocol("WM_DELETE_WINDOW", root.destroy)
    try:
        root.mainloop()
    except KeyboardInterrupt:
        print("\nKeyboard interrupt detected, closing application.")
        app.on_closing()