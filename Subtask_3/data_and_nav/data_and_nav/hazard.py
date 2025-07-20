#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from visualization_msgs.msg import Marker, MarkerArray
from pyproj import Proj, Transformer
import json
import requests

class Hazard_visualization(Node):
    def __init__(self, url):
        super().__init__('hazard_publisher')
        self.publisher=self.create_publisher(MarkerArray, '/visualization_marker_array', 10)
        self.timer=self.create_timer(5.0, self.publish_marker)

        self.url = url

        self.radii= [0.5, 1.0, 1.5]
        self.intensities=[1.0, 0.5, 0.2]
        self.transform=Transformer.from_crs("EPSG:4326", "EPSG:3857", always_xy=True)
        #ref_lon, ref_lat=
        #self.ref_x, self.ref_y= self.transform.transform(ref_lon, ref_lat)
        self.ref_x=0.0
        self.ref_y=0.0              

    def publish_marker(self):
        try: 
            header={'Accept': 'application/json'}
            response= requests.get(self.url, headers=header)
            data=response.json()

            with open('/tmp/hazard_data.json', 'w') as f:
                json.dump(data, f, indent=2)
            
            if data['status']=='OK':
                self.get_logger().warn(f"Server returned {data['status']}")

            else:
                self.get_logger().warn(f"Server returned {data['status']}")
                return


            marker_array=MarkerArray()
            marker_id=0

            f_destination=data.get('destination', [])
            lat=f_destination['lat']
            lon=f_destination['lon']
            #x, y=self.transform.transform(lat, lon)
            #rel_x= x-self.ref_x
            #rel_y=y-self.ref.y
            marker=Marker()
            marker.header.frame_id="map"
            marker.header.stamp=self.get_clock().now().to_msg()
            marker.id=marker_id
            marker_id+=1
            marker.type=Marker.SPHERE
            marker.action=Marker.ADD
            marker.pose.position.x=lat
            marker.pose.position.y=lon
            marker.pose.position.z=0.0
            marker.scale.x=marker.scale.y=marker.scale.z= 1.0
            marker.color.r=0.0
            marker.color.g=1.0
            marker.color.b=0.0
            marker.color.a=1.0
            marker_array.markers.append(marker)


            for l, hazard in enumerate(data.get('hazards', [])):
                lat=hazard['lat']
                lon=hazard['lon']
                #x, y=self.transform.transform(lat, lon)
                #rel_x= x-self.ref_x
                #rel_y=y-self.ref.y
                
                for m, (radius, intensity) in enumerate(zip(self.radii, self.intensities)):
                    marker=Marker()
                    marker.header.frame_id="map"
                    marker.header.stamp=self.get_clock().now().to_msg()
                    marker.id=marker_id
                    marker_id+=1
                    marker.type=Marker.SPHERE
                    marker.action=Marker.ADD
                    marker.pose.position.x=lat
                    marker.pose.position.y=lon
                    marker.pose.position.z=0.0
                    marker.scale.x=marker.scale.y=marker.scale.z= radius
                    marker.color.r=1.0
                    marker.color.g=0.0
                    marker.color.b=0.0
                    marker.color.a=intensity
                    marker_array.markers.append(marker)
                
            self.publisher.publish(marker_array)
            self.get_logger().info("Markers published successfully")
            
        except Exception as e:
            self.get_logger().error(f"Failed to fetch or parse data: {e}")
            
def main(args=None):
    rclpy.init(args=args)
    my_url=input("Enter URL and password: ")
    node=Hazard_visualization(my_url)
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()
            
        







            

        