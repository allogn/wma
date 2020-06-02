import sys, os
import numpy as np
import json

def processTwit(twit):
    try:
        time = twit['created_at']
        user = twit['user']['id']
        location = twit["coordinates"]
        if location != None and user != None and time != None and id != None:
            return {'coordinates': location, "user_id": user, "time": time}
        else:
            return None
    except KeyError:
        return None
    except TypeError:
        return None

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: path")
        exit()
    path = sys.argv[1]

    all_twits = []

    # all_twits_exists = False
    # for filename in os.listdir(path):
    #     if filename == "all_twits.json":
    #         all_twits_exists = True
    #         print("Previously merged.")
    #         break

    # if not all_twits_exists:
    for filename in os.listdir(path):
        if filename.split('.').pop() == "json":
            with open(path+"/"+filename,'r') as f:
                for line in f:
                    twit = json.loads(line)
                    processed = processTwit(twit)
                    if processed != None:
                        all_twits.append(processed)

    print("Collected " + str(len(all_twits)) + " tweets")
    json.dump(all_twits, open(path+"/"+"all_twits.json", "w"))
