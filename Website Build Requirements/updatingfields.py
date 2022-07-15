from flask import Flask, render_template, request, redirect, jsonify
# from flask_restful import Resource, Api, reqparse
import csv, bs4

filename = "locationdata.csv"
headersdict = {"LAT": "LATITUDE", "LONG": "LONGITUDE", "TIME": "CURRENT TIME", "ACCU": "ACCURACY LVL"}
# headerscsv = ["NAME", "LAT", "LONG", "TIME"]
headerscsv = list(headersdict.keys())
headerscopy = headerscsv.copy()
headerscopy.sort()

em_contacts = {"CON1": ["NAME1", "8925613813"], "CON2": ["NAME2", "8925613813"], "CON3": ["NAME3", "8925613813"]}

app = Flask(__name__)
username = "ADITYA MANOJ"
apikey = "myapikey"

# LOCATION TEMPLATE MODIFIER
def modifyTemplate():
    filename = "locationdata.csv"

    # load the file
    with open("templates/index.html") as inf:
        txt = inf.read()
        soup = bs4.BeautifulSoup(txt, features="html.parser")


    devinfo = soup.body.find(id="deviceinfo")
    devinfo.clear()
    deviceapikey = soup.new_tag("div", class_="p-2 bd-highlight")
    deviceapikey.string = "DEVICE ID: " + str(apikey)
    holdername = soup.new_tag("div", class_="p-2 bd-highlight")
    holdername.string = "HOLDER NAME: " + str(username)

    devinfo.append(deviceapikey)
    devinfo.append(holdername)

    emcontacts = soup.body.find(id="emcontacts")
    emcontacts.clear()
    em1 = soup.new_tag("div", class_="p-2 bd-highlight")
    em1.string = em_contacts["CON1"][0] + ": " + em_contacts["CON1"][1]
    em2 = soup.new_tag("div", class_="p-2 bd-highlight")
    em2.string = em_contacts["CON2"][0] + ": " + em_contacts["CON2"][1]
    em3 = soup.new_tag("div", class_="p-2 bd-highlight")
    em3.string = em_contacts["CON3"][0] + ": " + em_contacts["CON3"][1]

    emcontacts.append(em1)
    emcontacts.append(em2)
    emcontacts.append(em3)

    # MODIFYING CONTENT
    tablecontents = soup.body.find(id="tablecontents")
    loctable = tablecontents.find(id="loctable")

    # Finding empty head tag and replacing with content
    thead = loctable.find("thead")
    thead.clear()
    # if not loctable.find("thead"):
    # thead = soup.new_tag("thead")
    # loctable.append(thead)
    tr = soup.new_tag("tr")
    thead.append(tr)

    for val in headersdict.values():
        # val = headersdict[key]
        th = soup.new_tag("th")
        th.string = str(val)
        tr.append(th)

    tablecontents.find(id="loctable").find("tbody", id="tbody").clear()

    with open(filename, 'r') as csvfile:
        readerobj = csv.reader(csvfile)
        # fields = next(readerobj)


        for row in readerobj:
            # print(rows)
            new_link = soup.new_tag("tr")
            for item in row:
                tablecontents.find(id="loctable").find("tbody", id="tbody").append(new_link)
                value = soup.new_tag("th")
                value.string = str(item)
                new_link.append(value)


    # save the file again
    with open("templates/index.html", "w") as outf:
        outf.write(str(soup))


@app.route("/")
def hello_world():
    return "<p>Hello, World!</p>"


# Accepting Data from NodeMCU
@app.route('/post/' + apikey, methods=["POST"])
def testpost():
    input_json = request.get_json(force=True) 
    print(input_json)
    print(type(input_json))
    dictToReturn = input_json
    # dictToReturn = {'text':input_json['text']}
    # print(dictToReturn)
    # if dictToReturn.keys
    dictKeys = list(dictToReturn.keys())
    dictKeys.sort()
    # print(dictKeys)
    writeflag = False
    if dictKeys == headerscopy:
        with open(filename, 'a', newline="") as csvfile:
            dictwriterobj = csv.DictWriter(csvfile, fieldnames=headerscsv)
            dictwriterobj.writerow(dictToReturn)
            writeflag = True
            csvfile.close()
    
    if writeflag == True:
        return jsonify(dictToReturn)
    else:
        return "<h>DATA FORMAT MISMATCH</h>"

# Rejecting data from Unknown Sources
@app.route("/post", methods=["POST"])
def hello_world1():
    return "<h>REQUEST DENIED - API KEY MISSING</h>"

@app.route("/<string:pagename>")
def page(pagename):
    return render_template(pagename)


# Displaying stored location Data
@app.route('/getlocationdata/' + apikey)
def renderlocation():
    modifyTemplate()
    return render_template("index.html")
    # return render_template("index.html")
    # return beginhtml + endhtml

# app.run()