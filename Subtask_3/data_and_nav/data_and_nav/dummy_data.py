#!/usr/bin/env python3
from flask import Flask, request, jsonify

app = Flask(__name__)

@app.route("/data.json", methods=["GET"])
def serve_json():
    password= request.args.get("password")

    if password=="1234":
        return jsonify({
            'status': 'OK',
            'access_level': 'advanced',
            'destination': { 
                'lat': 20.678,
                'lon': 21.123456
            },
            'hazards': [ 
                {
                    'lat': 5.123456,
                    'lon': 6.123456,
                },
                {
                    'lat': 11.123456,
                    'lon': 2.123456,
                },
                {
                    'lat': -5.123456,
                    'lon': 15.123456,
                },
                {
                    'lat': 8.123456,
                    'lon': -2.123456,
                },

                
            ]
        })
    
    elif password=="5678":
        return jsonify({
            'status': 'OK',
            'access_level': 'basic',
            'destination': { 
                'lat': 20.678,
                'lon': 21.123456
            },
        })
    
    else: 
        return jsonify({
            'status': 'Error',
            'access_level': 'None',
        }), 401

def main():
    app.run(host="0.0.0.0", port=8000)

if __name__=="__main__":
    main()



        
