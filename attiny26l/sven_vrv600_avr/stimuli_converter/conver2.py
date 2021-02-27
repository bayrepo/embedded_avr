import sys
import csv
import ctypes

def PrintPORT(port, bit, value, ckl_sz, interval, ck):
    if interval>0:
        val = int(round(interval/ckl_sz))
        print("#%d"%(val))
        print("//%.12f" %(ck))
    res = ctypes.c_ubyte(1<<bit)
    if value == 0:
        res.value=~res.value
    dt=res.value
    if value == 0:
        print("%s &= %s"%(port, hex(dt)))
    else:
        print("%s |= %s"%(port, hex(dt)))

#example usage: py convert.py 8000000 test.csv output_file 10
#Time[s],  PINB 2,  PINB 0
#-0.012501375, 0, 1
#0.000000000, 1, 1
#0.000010250, 1, 0
#0.000020125, 0, 0

f_cpu = sys.argv[1]
f_name = sys.argv[2]
f_output = sys.argv[3]
start_cycle = sys.argv[4]


try:
    f_cpu = int(f_cpu)
except ValueError:
    f_cpu = 8000000

print("F_CPU %d from file %s" %(f_cpu, f_name))

nclk = 1/f_cpu
index = 0
print("1 cyckle size %.12f" %(nclk))

orig_stdout = sys.stdout
f = open(f_output, 'w')
sys.stdout = f

data = []
data_dict = {}
pins = {}
pins_names = {}
bit_names = {}
val_names = {}
last_cycle = 0

with open(f_name, newline='') as csvfile:
     spamreader = csv.reader(csvfile, delimiter=',', quotechar='|')
     for row in spamreader:
        if index == 0:
            index=index+1
            row_int = 0
            for item in row:
              item = item.strip()
              if item.startswith('PIN'):
                pins[item]=row_int
                pins_names[row_int]=item.split(" ")[0]
                bit_names[row_int]=int(item.split(" ")[1])
              row_int = row_int + 1
            continue
        else:
          if index == 1:
            for k, v in pins_names.items():
              if v not in val_names.keys():
                val_names[v]=0
          flt = float(row[0])
          dd = row[0]
          if flt<0.0:
            dd = "-1"
          data.append(dd)
          ln = len(row)
          ind = 1
          pn = {}
          while(ind<ln):
            if (pins_names[ind] in pn.keys()):
              pn[pins_names[ind]][bit_names[ind]] = int(row[ind])
            else:
              pn[pins_names[ind]] = {}
              pn[pins_names[ind]][bit_names[ind]] = int(row[ind])
            ind = ind + 1
          data_dict[dd] = pn
        index=index+1

last_clk = int(start_cycle)
cur_pins = {}
pins_names_u = list(set(pins_names.values()))

for k in data:
  if k == "-1":
    for key in pins_names_u:
      if key not in cur_pins.keys():
        cur_pins[key]="--------"
      lst = list(cur_pins[key])
      for pk in data_dict[k][key].keys():
        if data_dict[k][key][pk] == 0:
          lst[pk]='0'
        else:
          lst[pk]='1'
      cur_pins[key]="".join(lst)
    fst = 0
    for key in pins_names_u:
      print("//%s=%s" %(key, cur_pins[key]))
    for ckey in cur_pins.keys():
      l = list(cur_pins[ckey])
      cnt = 0
      for v in l:
        if v == '-':
          continue
        if fst == 0: 
          PrintPORT(ckey, cnt, int(v), nclk, -last_clk * nclk, 0)
          fst = 1
        else:
          PrintPORT(ckey, cnt, int(v), nclk, 0.0, 0)
        cnt = cnt + 1;
    print("#%d"%(last_clk))
    last_clk = 0
  else:
    ckl = float(k)
    new_pins = {}
    for key in pins_names_u:
      if key not in new_pins.keys():
        new_pins[key]="--------"
      lst = list(new_pins[key])
      for pk in data_dict[k][key].keys():
        if data_dict[k][key][pk] == 0:
          lst[pk]='0'
        else:
          lst[pk]='1'
      new_pins[key]="".join(lst)
    fst = 0
    for key in pins_names_u:
      print("//%s=%s" %(key, new_pins[key]))
    for key in pins_names_u:
      if cur_pins[key] != new_pins[key]:
        l = list(new_pins[key])
        cnt = 0
        for v in l:
          if v == '-':
            continue
          if fst == 0: 
            PrintPORT(key, cnt, int(v), nclk, ckl - last_clk , ckl)
            fst = 1
          else:
            PrintPORT(key, cnt, int(v), nclk, 0.0, ckl)
          cnt = cnt + 1;
    cur_pins = new_pins
    if fst == 1:
      last_clk = ckl

sys.stdout = orig_stdout
f.close()