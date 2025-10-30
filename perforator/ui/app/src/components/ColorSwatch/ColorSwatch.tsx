import './ColorSwatch.css';


export const ColorSwatch: React.FC<{color: string}> = ({ color }) => <div style={{ backgroundColor: color }} className="color-swatch"></div>;
