#!/usr/bin/env python3
import tkinter as tk
from tkinter import ttk
from PIL import Image, ImageTk
import subprocess
import os
import json


# Ensure the current working directory is set to the script's directory
script_dir = os.path.dirname(os.path.abspath(__file__))
os.chdir(script_dir)    


class GraphPropertiesForm:
    def __init__(self, master):
        self.master = master
        self.mst_process = None
        master.title("Graph Properties")
        master.geometry("350x666")
        master.resizable(False, False)

        bg_image_file = Image.open("visual/visual.png")
        bg_image_file = bg_image_file.resize((350, 666), Image.Resampling.LANCZOS)
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
        seed_entry.place(x=160, y=110)

        # Nodes Entry
        self.nodes_var = tk.StringVar(value="64")
        nodes_entry = tk.Entry(master, textvariable=self.nodes_var, font=entry_font, width=entry_width, justify='right',
                      bg=entry_bg, relief=entry_relief, borderwidth=entry_borderwidth, highlightthickness=0, disabledbackground='#fff')
        nodes_entry.place(x=160, y=172)

        # Edges Entry
        self.edges_var = tk.StringVar(value="256")
        edges_entry = tk.Entry(master, textvariable=self.edges_var, font=entry_font, width=entry_width, justify='right',
                      bg=entry_bg, relief=entry_relief, borderwidth=entry_borderwidth, highlightthickness=0, disabledbackground='#fff')
        edges_entry.place(x=160, y=234)

        # Density Entry
        self.density_var = tk.StringVar(value="0.12698")
        density_entry = tk.Entry(master, textvariable=self.density_var, font=entry_font, width=entry_width, justify='right',
                      bg=entry_bg, relief=entry_relief, borderwidth=entry_borderwidth, highlightthickness=0, disabledbackground='#fff')
        density_entry.place(x=160, y=296)
        density_entry.config(state='disabled')

        # --- Checkboxes ---
        self.connected_var = tk.BooleanVar(value=True)
        connected_check = tk.Checkbutton(master, variable=self.connected_var, bg='#f0f0f0',
               activebackground="#ffffff", activeforeground="#000000", indicatoron=False,
               selectcolor="#77ccff",height=2, width=5,highlightbackground="#f0f0f0", highlightthickness=3)
        connected_check.place(x=195, y=344)

        self.comlpete_var = tk.BooleanVar()
        comlpete_check = tk.Checkbutton(master, variable=self.comlpete_var, bg='#f0f0f0',
               activebackground="#ffffff", activeforeground="#000000",  indicatoron=False,
               selectcolor="#77ccff",height=2, width=5,highlightbackground="#f0f0f0", highlightthickness=3)    
        comlpete_check.place(x=195, y=406)

        self.regular_var = tk.BooleanVar()
        regular_check = tk.Checkbutton(master, variable=self.regular_var, bg='#f0f0f0',
               activebackground="#ffffff", activeforeground="#000000",  indicatoron=False,
               selectcolor="#77ccff",height=2, width=5,highlightbackground="#f0f0f0", highlightthickness=3)
        regular_check.place(x=195, y=468)

        # --- Buttons ---

        self.submit_btn = tk.Button(master, text="Submit", command=self.submit_action, font=('Helvetica', 10, 'bold'), relief=tk.RIDGE, borderwidth=0,
                                    padx=19, pady=17, bg='#ddd', activeforeground='#000000')
        self.submit_btn.place(x=80, y=544)

        self.confirm_btn = tk.Button(master, text="",  font=('Helvetica', 10, 'bold'), relief=tk.RIDGE, borderwidth=0,
                                    padx=19, pady=17, width=9, bg='#ddd', activeforeground='#000000', )
        self.confirm_btn.place(x=200, y=593)
        self.submit_btn.config(state=tk.NORMAL)
        self.confirm_btn.config(state=tk.DISABLED)

        self.nodes_var.trace_add('write', self.update_density)
        self.edges_var.trace_add('write', self.update_density)
        # self.density_var.trace_add('write', self.update_edges)


    def update_density(self, *args):
        try:
            nodes = int(self.nodes_var.get())
            edges = int(self.edges_var.get())
            if nodes > 1:
                density = edges * 2 / (nodes * (nodes - 1))
                self.density_var.set(f"{density:.5f}")
        except ValueError:
            pass  # Ignore invalid input

    # def update_edges(self, *args):
    #     try:
    #         nodes = int(self.nodes_var.get())
    #         density = float(self.density_var.get())
    #         if nodes > 1:
    #             max_edges = nodes * (nodes - 1) // 2
    #             edges = int(density * max_edges)
    #             self.edges_var.set(str(edges))
    #     except ValueError:
    #         pass  # Ignore invalid input
        

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
            "seed": self.seed_var.get() if self.seed_var.get() else "0",
            "nodes": self.nodes_var.get() if self.nodes_var.get() else "64",
            "edges": self.edges_var.get()  if self.edges_var.get() else "256",
            "density": self.density_var.get() if self.density_var.get() else "0.05",
            "connected": self.connected_var.get(),
            "complete": self.comlpete_var.get(),
            "regular": self.regular_var.get(),
        }
        #show taken values at the fields
        self.seed_var.set(self.data["seed"])
        self.nodes_var.set(self.data["nodes"])
        self.edges_var.set(self.data["edges"])
        self.density_var.set(self.data["density"])  
        # Ckeck for existance of input_graph.txt

        filename = f"input/{self.data['nodes']}_{self.data['edges']}_{self.data['seed']}_{int(self.data['connected'])}{int(self.data['complete'])}{int(self.data['regular'])}.txt"

        if not os.path.exists (filename):
            self.confirm_btn.config(state=tk.NORMAL, text="Create", command=self.create_action)
        else:
            #check if matrix has same meta data with success = 1 and other parameters same
            with open("input_params.json", "r") as f:
                existing_data = f.read()
                if existing_data.strip() == "\n".join(f"{key}: {value}" for key, value in self.data.items()):
                    self.confirm_btn.config(state=tk.NORMAL, text="MST", command=self.confirm_action)
                else:
                    self.confirm_btn.config(state=tk.NORMAL, text="Create", command=self.create_action)

        #make yourself unsubmit
        self.submit_btn.config(text=" Edit ", state=tk.NORMAL , command=self.edit_action)

    def edit_action(self):
        self.submit_btn.config(text="Submit", command=self.submit_action)
        # Enable all entries and checkboxes
        for widget in self.master.winfo_children():
            if isinstance(widget, tk.Entry):
                if widget.winfo_id() != 37748793:  # density entry
                    # print(widget.info)
                    widget.config(state=tk.NORMAL)
            elif isinstance(widget, tk.Checkbutton):
                widget.config(state=tk.NORMAL)
        
        self.confirm_btn.config(state=tk.DISABLED, text="")
                    

    def create_action(self):
        with open("input_params.json", "w") as f:
            json.dump(self.data, f, indent=4)
        self.confirm_btn.config(state=tk.DISABLED, text="creating...", command=self.confirm_action)

        result = subprocess.run(["make", "matrix" ,], cwd=script_dir, capture_output=False, text=True, check=True)

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
                if widget.winfo_id() != 37748793:  # density entry
                    widget.config(state=tk.NORMAL)
            elif isinstance(widget, tk.Checkbutton):
                widget.config(state=tk.NORMAL)
        self.confirm_btn.config(text="MST_ing")
        print("Get MST")
        self.mst_process = subprocess.Popen(["make", "mst", "--silent"], cwd=script_dir)
        
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