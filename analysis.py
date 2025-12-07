import csv
import re
import math
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

input_file = "track.txt"
output_file = "output.csv"

def convert_to_csv():
    entries = []
    current = {}
    fields = []            # ordered list of columns
    started = False        # becomes True after first dashed line

    with open(input_file, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()

            # Start reading only after the first dashed line
            if not started:
                if line.startswith("-"):   # first separator found
                    started = True
                continue
            
            # Skip empty lines and separators
            if not line or line.startswith("-"):
                # If a block ended, save the current dict
                if current:
                    entries.append(current)
                    current = {}
                continue

            # Extract key-value pairs
            if ":" in line:
                key, value = line.split(":", 1)
                key = key.strip()            #Removes leading and trailing characters
                value = value.strip()
                current[key] = value

                # Maintain field order (no duplicates)
                if key not in fields:
                    fields.append(key)

    # Write to CSV
    with open(output_file, "w", newline="", encoding="utf-8") as csv_file:            # "newline=''" prevents extra blank lines from appearing in the CSV
        writer = csv.DictWriter(csv_file, fieldnames=fields)          # What is DictWriter? A helper from Python’s csv module that lets us write rows using dictionaries
        writer.writeheader()
        writer.writerows(entries)

convert_to_csv()

'''
Step1: Start
Step2: Import re and csv modules
Step3: Store input and output filenames in variables
Step4: Open the .txt file for reading
Step5: Begin iterating line by line; start processing only after the first dashed line (--------)
Step6: If a line is empty or another dashed line:
    If current block has data, append it to entries
    Reset current dictionary
    Continue to next line
Step7: If a line contains : then:
    Split into key and value
    Strip spaces from both
    Store in current[key] = value
Step8: Maintain an ordered fields list:
    If the key is not in fields, append it
Step9: After the loop, append any remaining current block to entries
Step10: Open the CSV file
Step11: Write fields as the header and all entries as rows
Step12: End
'''


"""
DATA HANDLING RULES (SUMMARY)

1. Missing values are replaced using a "worst-case default" strategy:
      - For numeric categories (e.g., exercise, LeetCode reps):
            Missing → 0 
            (0 is the minimum realistic value a person can achieve)
      - For binary categories (e.g., facepack, skincare):
            Missing → "No"

2. Purpose of this strategy:
      - Avoids giving users artificially inflated performance.
      - Ensures graphs remain continuous and predictable.
      - Encourages users to fill missing days to avoid worst-case defaults.

3. Category Types:
      - Category 1 (secure 3-value category):
            Use only allowed values {1,2,3}; invalid → null; ignore null in plots.
      - Category 2 (exercise/productivity numeric):
            Treat all values as integers; missing → 0.

4. Visualization:
      - Category 1 → bar chart (ignore nulls)
      - Category 2 → line chart + optional weekly bar chart

5. Date ranges:
      - Use fixed windows (7–10 days) instead of arbitrary ranges
        to avoid bugs and ensure stable plotting behavior.

"""

def analyze_data():
    # loading the data
    dataset = pd.read_csv("output.csv");

    # Analyzing numeric data
    dataset["Date"] = pd.to_datetime(dataset["Date"], dayfirst=True)               # Convert date into datetime
    numeric_cols = dataset.select_dtypes(include='number').columns
    plt.figure(figsize=(12, 6))
    markers = ['o', 's', 'D', '^', 'v', 'P', '*', 'X', '<', '>']             # unique marker list
    offset_strength = 0.1              # tiny offset to avoid total overlap when values are all zeros
    for i,col in enumerate(numeric_cols):
        values = dataset[col].astype(float)
        x = dataset["Date"]              # dates on x-axis
        offset = i * offset_strength
        adjusted_values = values + offset
        plt.plot(
            x,
            adjusted_values,
            marker=markers[i % len(markers)],   # unique marker
            markersize=7,
            linewidth=2,
            label=col
        )
    plt.title("Daily Progress")
    plt.xlabel("Date")
    plt.ylabel("Values")
    plt.grid(True)
    plt.legend()
    plt.xticks(rotation=45)  # Rotate dates for visibility
    plt.tight_layout()
    plt.show()

    # Analyzing string data
    string_cols = dataset.select_dtypes(include='object').columns
    dataset[string_cols] = dataset[string_cols].apply(lambda col: col.str.strip().str.lower())
    string_cols = [col for col in string_cols if col != "Date"]        # Removes the "Date" column since it is not a category to count.
    # Number of subplots needed
    n = len(string_cols)                 # Counts how many string columns you have.
    # Create rows and columns for grid layout
    cols = 2                                  # 2 columns looks neat
    rows = math.ceil(n / cols)                # calculates required rows
    fig, axes = plt.subplots(rows, cols, figsize=(12, 4 * rows))
    axes = axes.flatten()  # Flatten for easy indexing
    color_map = {
        "yes": "green",
        "no": "red",
        "kaza": "yellow",
    }
    for i, col in enumerate(string_cols):
        counts = dataset[col].value_counts(dropna=False)
        categories = counts.index.tolist()
        values = counts.values
        x = np.arange(len(categories))        # convert categories to numeric positions
        spacing = 1.0  
        x = x * spacing
        colors = [color_map.get(cat, "blue") for cat in categories]        # Assign colors (fallback to blue if category not in color_map)
        bar_width = 0.5            # FIXED BAR WIDTH
        axes[i].bar(x, values, color=colors, width=bar_width)
        axes[i].set_title(col)
        axes[i].set_ylabel("Count")
        axes[i].set_xticks(x)
        axes[i].set_xticklabels(categories, rotation=30, fontsize=9)
        axes[i].set_xlim(-0.7, x[-1] + 0.7)         # Keeps bars visually fixed-width
    # Hide unused subplot boxes (if any)
    for j in range(i+1, len(axes)):
        axes[j].set_visible(False)
    plt.subplots_adjust(hspace=0.4, wspace=0.3, bottom=0.05,top=0.95)
    plt.show()

analyze_data();