from collections import defaultdict
def compareCsv(df1, df2):
    truth_map = defaultdict(int)
    comp_map = defaultdict(int)
    for index, rows in df1.iterrows():
        truth_map["_".join(map(str, rows.tolist()))]+=1
    for index, rows in df2.iterrows():
        comp_map["_".join(map(str, rows.tolist()))]+=1
    # print(truth_map)
    # print(comp_map)   
    if(len(truth_map) != len(comp_map)):
        return False 
    for k in truth_map:
        if(k not in comp_map):
            print(k)
            return False 
        if(truth_map[k]!=comp_map[k]):
            return False 
    return True 