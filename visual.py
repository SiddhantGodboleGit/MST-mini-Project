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

        def validate_int(P):
            return P.isdigit() or P == ""
        vcmd = (master.register(validate_int), '%P')

        self.seed_var = tk.StringVar(value="0")
        seed_entry = tk.Entry(master, textvariable=self.seed_var, font=entry_font, width=entry_width, justify='right',
                      bg=entry_bg, relief=entry_relief, borderwidth=entry_borderwidth, highlightthickness=0, disabledbackground='#fff',
                      validate='key', validatecommand=vcmd)
        seed_entry.place(x=160, y=109)

        # Nodes Entry
        self.nodes_var = tk.StringVar(value="64")
        nodes_entry = tk.Entry(master, textvariable=self.nodes_var, font=entry_font, width=entry_width, justify='right', 
                      bg=entry_bg, relief=entry_relief, borderwidth=entry_borderwidth, highlightthickness=0, disabledbackground='#fff',
                      validate='key', validatecommand=vcmd)
        nodes_entry.place(x=160, y=171)

        # Edges Entry
        self.edges_var = tk.StringVar(value="256")
        edges_entry = tk.Entry(master, textvariable=self.edges_var, font=entry_font, width=entry_width, justify='right',
                      bg=entry_bg, relief=entry_relief, borderwidth=entry_borderwidth, highlightthickness=0, disabledbackground='#fff',
                      validate='key', validatecommand=vcmd)
        edges_entry.place(x=160, y=233)

        # Threads Entry
        self.thread_var = tk.StringVar(value="4")
        thread_entry = tk.Entry(master, textvariable=self.thread_var, font=entry_font, width=entry_width, justify='right',
                      bg=entry_bg, relief=entry_relief, borderwidth=entry_borderwidth, highlightthickness=0, disabledbackground='#fff',
                      validate='key', validatecommand=vcmd)
        thread_entry.place(x=160, y=295)

        # --- Checkboxes ---
        self.connected_var = tk.BooleanVar(value=True)
        connected_check = tk.Checkbutton(master, variable=self.connected_var, bg='#f0f0f0',
               activebackground="#ffffff", activeforeground="#000000", indicatoron=False,
               selectcolor="#77ccff",height=2, width=5,highlightbackground="#f0f0f0", highlightthickness=3)
        connected_check.place(x=195, y=343)

        self.comlpete_var = tk.BooleanVar()
        comlpete_check = tk.Checkbutton(master, variable=self.comlpete_var, bg='#f0f0f0',
               activebackground="#ffffff", activeforeground="#000000",  indicatoron=False,
               selectcolor="#77ccff",height=2, width=5,highlightbackground="#f0f0f0", highlightthickness=3)    
        comlpete_check.place(x=195, y=405)

        self.regular_var = tk.BooleanVar()
        regular_check = tk.Checkbutton(master, variable=self.regular_var, bg='#f0f0f0',
               activebackground="#ffffff", activeforeground="#000000",  indicatoron=False,
               selectcolor="#77ccff",height=2, width=5,highlightbackground="#f0f0f0", highlightthickness=3)
        regular_check.place(x=195, y=467)

        # --- Buttons ---

        self.submit_btn = tk.Button(master, text="Submit", command=self.submit_action, font=('Helvetica', 10, 'bold'), relief=tk.RIDGE, borderwidth=0,
                                    padx=19, pady=17, bg='#ddd', activeforeground='#000000')
        self.submit_btn.place(x=80, y=543)

        self.confirm_btn = tk.Button(master, text="",  font=('Helvetica', 10, 'bold'), relief=tk.RIDGE, borderwidth=0,
                                    padx=19, pady=17, width=9, bg='#ddd', activeforeground='#000000', )
        self.confirm_btn.place(x=200, y=592)
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
            "seed": self.seed_var.get() if self.seed_var.get() else "0",
            "nodes": self.nodes_var.get() if self.nodes_var.get() else "64",
            "edges": self.edges_var.get()  if self.edges_var.get() else "256",
            "threads": self.thread_var.get() if self.thread_var.get() else "4",
            "connected": self.connected_var.get(),
            "complete": self.comlpete_var.get(),
            "regular": self.regular_var.get(),
        }
        #show taken values at the fields
        self.seed_var.set(self.data["seed"])
        self.nodes_var.set(self.data["nodes"])
        self.edges_var.set(self.data["edges"])
        self.thread_var.set(self.data["threads"])
        # Ckeck for existance of input_graph.txt
        connected_val = "1" if self.data['connected'] else "0"
        complete_val = "1" if self.data['complete'] else "0"
        regular_val = "1" if self.data['regular'] else "0"

                # Debug: Print the converted values
        # print(f"Debug - connected_val: {connected_val}")
        # print(f"Debug - complete_val: {complete_val}")
        # print(f"Debug - regular_val: {regular_val}")
        
        filename = f"input/{self.data['nodes']}_{self.data['edges']}_{self.data['seed']}_{connected_val}_{complete_val}_{regular_val}.txt"

        with open("input_params.json", "w") as f:
            json.dump(self.data, f, indent=4)

        # filename = f"input/{self.data['nodes']}_{self.data['edges']}_{self.data['seed']}_{bool(self.data['connected'])}{bool(self.data['complete'])}{bool(self.data['regular'])}.txt"

        if os.path.exists (filename):
            self.confirm_btn.config(state=tk.NORMAL, text="MST", command=self.confirm_action)
        else:
            self.confirm_btn.config(state=tk.NORMAL, text="Create", command=self.create_action)

        #make yourself unsubmit
        self.submit_btn.config(text=" Edit ", state=tk.NORMAL , command=self.edit_action)

    def edit_action(self):
        self.confirm_btn.config(state=tk.DISABLED, text="")
        self.submit_btn.config(text="Submit", command=self.submit_action)
        # Enable all entries and checkboxes
        for widget in self.master.winfo_children():
            if isinstance(widget, tk.Entry):
                widget.config(state=tk.NORMAL)
            elif isinstance(widget, tk.Checkbutton):
                widget.config(state=tk.NORMAL)
        # Kill the subprocess if it's running
        # if self.mst_process:
        #     self.mst_process.terminate()
        #     self.mst_process = None

    def create_action(self):
        self.confirm_btn.config(state=tk.DISABLED, text="creating...", command=self.confirm_action)
        print("\033[1mmake graph\033[0m --silent")
        result = subprocess.run(["make", "graph" , "--silent"], cwd=script_dir, capture_output=False, text=True, check=True)
        self.confirm_btn.config(state=tk.NORMAL, text="MST", command=self.confirm_action)


    def confirm_action(self):
        self.submit_btn.config(state=tk.NORMAL)
        self.confirm_btn.config(state=tk.DISABLED)
        # reactivate all entries and checkboxes
        #kill all other processes
        # if self.mst_process:
        #     self.mst_process.terminate()
        #     self.mst_process = None
        # for widget in self.master.winfo_children():
        #     if isinstance(widget, tk.Entry):
        #         if widget.winfo_id() != 37748793:  # density entry
        #             widget.config(state=tk.NORMAL)
        #     elif isinstance(widget, tk.Checkbutton):
        #         widget.config(state=tk.NORMAL)
        self.confirm_btn.config(text="MST_ing")
        print("\033[1mmake mst\033[0m --silent")
        # self.mst_process = subprocess.Popen(["make", "mst", "--silent"], cwd=script_dir)
        self.mst_process = subprocess.run(["make", "mst", "--silent"], cwd=script_dir, capture_output=False, text=True, check=True)
        self.confirm_btn.config(state=tk.NORMAL, text="Done", command=self.on_closing)

    def on_closing(self):
        # Kill the subprocess when the main window is closed
        # if self.mst_process and self.mst_process.poll() is None:
        #     self.mst_process.kill()
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