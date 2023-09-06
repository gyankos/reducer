import os
from pathlib import Path
import pandas

model = 'Clauses'
reduced = 'Reduced'
conjoined = 'Generation'

def get_data_deriver(s:str)->str:
    if s == "cr":
        return "ChainResponse"
    elif s == "cr2":
        return "ChainResponseAX"
    elif s == "p":
        return "Precedence"
    elif s == "r":
        return "Response"
    elif s == "re":
        return "RespExistence+Exists"
    elif s == "rexc":
        return "RespExistence+ExclChoice+Exists"
    elif s == "crar":
        return "(Chain+Alt)Response"

def determine_line(s: str):
    ls = s.split("_")
    return (ls[0], get_data_deriver(ls[1]), ls[-1], " ".join(ls[2:-1]))

def determine_algo(s:str)->str:
    if s == "aaltaf_axmodel":
        return "AALTAF+AX"
    elif s == "aaltaf_model":
        return "AALTAF"
    elif s == "aaltaf_rmodel":
        return "AALTAF(R)"
    elif s == "axmodel":
        return "LYDIA+AX"
    elif s == "model":
        return "LYDIA"
    elif s == "rmodel":
        return "LYDIA(R)"
    else:
        return s

def parse_out_line(file, isThirdLine = True, absThird = None):
    DATASET = []
    CONF = []
    ORIG = []
    CONJOINED = []
    ALGORITHM = []
    TIME = []
    APPEND = []
    with open(file, "r") as f:
        for line in f:
            line = line.rstrip()
            l = line.split(",")
            (dataset,conf,orig,  cj) = determine_line(l[0])
            DATASET.append(dataset)
            CONF.append(conf)
            ORIG.append(orig)
            CONJOINED.append(cj)
            ALGORITHM.append(determine_algo(l[1]))
            TIME.append(l[2])
            if isThirdLine:
                APPEND.append(l[3])
    if isThirdLine:
        return pandas.DataFrame(data={"dataset":DATASET,
                                      model:CONF,
                                      "orig":ORIG,
                                      conjoined:CONJOINED,
                                      "algorithm":ALGORITHM,
                                      "OOM":APPEND,
                                      "time":TIME})
    else:
        d = pandas.DataFrame(data={"dataset":DATASET,
                                      model:CONF,
                                      "orig":ORIG,
                                      conjoined:CONJOINED,
                                      "algorithm":ALGORITHM,
                                      "time":TIME})
        d = pandas.merge(left=d, right=absThird, on=["dataset", model, "orig", conjoined, "algorithm"], how='outer')
        d = d.dropna(subset=['time'])
        d.OOM = d.OOM.fillna(False)
        return d

def parse_knobab_file(f):
    k = pandas.read_csv(f)
    k = k.drop('with_data', axis=1)
    k['log_filename'] = k['log_filename'].apply(lambda x: Path(x).stem)
    DATASET = []
    CONF = []
    ORIG = []
    CONJOINED = []
    REDUCED = []
    loading = k[k['model_ltlf_query_time'] == -1]
    mining = k[k['model_ltlf_query_time'] != -1]
    mining = mining.drop('log_loading_and_parsing_ms', axis=1)\
        .drop('log_indexing_ms', axis=1)\
        .drop('n_traces', axis=1)\
        .drop('n_acts', axis=1)\
        .drop('log_trace_average_length', axis=1)\
        .drop('log_trace_variance', axis=1)\
        .drop('most_frequent_trace_length', axis=1)\
        .drop('trace_length_frequency', axis=1)\
        .drop('atomization_conf', axis=1)\
        .drop('model_parsing_ms', axis=1)\
        .drop('iteration_num', axis=1)\
        .drop('min_support', axis=1)\
        .drop('scheduling_strategy', axis=1)\
        .drop('no_threads', axis=1)\
        .drop('is_multithreaded', axis=1)\
        .drop('queries_plan', axis=1)\
        .drop('operators_version', axis=1)\
        .drop('mining_algorithm', axis=1)

    mining['total_time'] = (mining['model_data_decomposition_time'])+(mining['model_atomization_time'])+(mining['model_declare_to_ltlf'])+(mining['model_ltlf_query_time'])
    mining = mining.drop('model_data_decomposition_time',axis=1)\
        .drop('model_atomization_time', axis=1)\
        .drop('model_declare_to_ltlf', axis=1)\
        .drop('model_ltlf_query_time', axis=1)
    mining = mining.groupby(["log_filename", "model_filename"],as_index=False).sum().fillna(0)

    for x in mining["model_filename"]:
        (dataset,conf,orig,cj) = determine_line(os.path.basename(os.path.dirname(x)))
        DATASET.append(dataset)
        CONF.append(conf)
        ORIG.append(orig)
        CONJOINED.append(cj)
        REDUCED.append("reduced" in Path(x).stem)

    mining = mining.drop('model_filename', axis=1).join(pandas.DataFrame({'dataset': DATASET,
                                model: CONF,
                                'orig':ORIG,
                                conjoined:CONJOINED,
                                reduced:REDUCED}))[['log_filename', 'model_size', 'dataset', model, 'orig', conjoined, reduced, 'total_time']]
    mining = mining.sort_values(by=mining.columns.to_list(),ascending=True)
    return (loading,mining)

def parse_data_element(s:str, d : list):
    lines = s.lstrip().split('\n')
    header = Path(lines[0]).stem
    dataset,conf,orig,  cj = determine_line(header)
    lydia = False
    lydia_ax = False
    lydia_r = False
    aaltaf = False
    aaltaf_ax = False
    aaltaf_r = False
    # if orig == "32,model":
    #     print("ERROR")
    for idx, x in enumerate(lines):
        if "[lydia]" in x:
            if "ltlf_ax_model" in x:
                lydia_ax = True
                if "Time elapsed for DFA construction:" in x:
                    d.append([dataset,conf,orig,  cj, determine_algo("axmodel"), False])
                else:
                    d.append([dataset,conf,orig,  cj, determine_algo("axmodel"), True])
            if "ltlf_model.txt" in x:
                lydia = True
                if "Time elapsed for DFA construction:" in x:
                    d.append([dataset,conf,orig,  cj, determine_algo("model"), False])
                else:
                    d.append([dataset,conf,orig,  cj, determine_algo("model"), True])
            if "ltlf_reduced.txt" in x:
                lydia_r = True
                if "Time elapsed for DFA construction:" in x:
                    d.append([dataset,conf,orig,  cj, determine_algo("rmodel"), False])
                else:
                    d.append([dataset,conf,orig,  cj, determine_algo("rmodel"), True])
        elif "aaltaf_" in x:
            if "aaltaf_ax_model" in x:
                aaltaf_ax = True
                if "b''" == lines[idx+1]:
                    d.append([dataset,conf,orig,  cj, determine_algo("aaltaf_axmodel"), True])
                else:
                    d.append([dataset,conf,orig,  cj, determine_algo("aaltaf_axmodel"), False])
            if "aaltaf_model.txt" in x:
                aaltaf = True
                if "b''" == lines[idx+1]:
                    d.append([dataset,conf,orig,  cj, determine_algo("aaltaf_model"), True])
                else:
                    d.append([dataset,conf,orig,  cj, determine_algo("aaltaf_model"), False])
            if "aaltaf_reduced.txt" in x:
                aaltaf_r = True
                if "b''" == lines[idx+1]:
                    d.append([dataset,conf,orig,  cj, determine_algo("aaltaf_rmodel"), True])
                else:
                    d.append([dataset,conf,orig,  cj, determine_algo("aaltaf_rmodel"), False])
    if not aaltaf_r:
        d.append([dataset, conf, orig, cj, determine_algo("aaltaf_rmodel"), True])
    if not aaltaf:
        d.append([dataset, conf, orig, cj, determine_algo("aaltaf_model"), True])
    if not aaltaf_ax:
        d.append([dataset, conf, orig, cj, determine_algo("aaltaf_axmodel"), True])
    if not lydia_r:
        d.append([dataset, conf, orig, cj, determine_algo("rmodel"), True])
    if not lydia:
        d.append([dataset, conf, orig, cj, determine_algo("model"), True])
    if not lydia_ax:
        d.append([dataset, conf, orig, cj, determine_algo("axmodel"), True])
    return d



if __name__ == '__main__':
    data = None
    with open('/home/giacomo/projects/reducer/oom_file.log_OK.txt', 'r') as file:
        data = file.read()
    d = list()
    # d = parse_data_element(data.split('\n\n')[-2], d)
    for x in data.split('\n\n'):
        if (len(x.strip()))==0: continue
        d = parse_data_element(x, d)
    d = pandas.DataFrame(d)
    d.columns = ["dataset", model, "orig", conjoined, "algorithm", "OOM"]
    d2 = parse_out_line("/home/giacomo/projects/reducer/out.csv", False, d)
    d2.to_csv("/home/giacomo/projects/reducer/rest_running.csv", index=False)

    d = parse_out_line("/home/giacomo/projects/reducer/out2.csv")
    d.to_csv("/home/giacomo/projects/reducer/rest_running2.csv",index=False)
    loading, mining = parse_knobab_file("/home/giacomo/projects/reducer/knobab.csv")
    loading.to_csv("/home/giacomo/projects/reducer/knobab_loading.csv",index=False)
    mining.to_csv("/home/giacomo/projects/reducer/knobab_formal.csv",index=False)